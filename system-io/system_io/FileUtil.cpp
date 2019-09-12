#include "system_io/FileUtil.h"
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <system_error>
#include <vector>
#include "system_io/ScopeGuard.h"


namespace sysio
{
    inline void incr(ssize_t /* n */) {}
    inline void incr(ssize_t n, off_t& offset) {
        offset += off_t(n);
    }

    int openNoInt(const char *name, int flags, mode_t mode)
    {
        return int(wrapNoInt(open, name, flags, mode));
    }

    static int filterCloseReturn(int r)
    {
        // Ignore EINTR.  On Linux, close() may only return EINTR after the file
        // descriptor has been closed, so you must not retry close() on EINTR --
        // in the best case, you'll get EBADF, and in the worst case, you'll end up
        // closing a different file (one opened from another thread).
        //
        // Interestingly enough, the Single Unix Specification says that the state
        // of the file descriptor is unspecified if close returns EINTR.  In that
        // case, the safe thing to do is also not to retry close() -- leaking a file
        // descriptor is definitely better than closing the wrong file.
        if (r == -1 && errno == EINTR)
        {
            return 0;
        }
        return r;
    }

    int closeNoInt(int fd)
    {
        return filterCloseReturn(close(fd));
    }

    int dupNoInt(int fd)
    {
        return int(wrapNoInt(dup, fd));
    }

    int dup2NoInt(int oldFd, int newFd)
    {
        return int(wrapNoInt(dup2, oldFd, newFd));
    }

    int flockNoInt(int fd, int operation)
    {
        return int(wrapNoInt(flock, fd, operation));
    }

    ssize_t readNoInt(int fd, void *buf, size_t count)
    {
        return wrapNoInt(read, fd, buf, count);
    }

    ssize_t writeNoInt(int fd, const void *buf, size_t count)
    {
        return wrapNoInt(write, fd, buf, count);
    }

    ssize_t readFull(int fd, void* buf, size_t count) {
        return wrapFull(read, fd, buf, count);
    }

    ssize_t writeFull(int fd, const void* buf, size_t count) {
        return wrapFull(write, fd, const_cast<void*>(buf), count);
    }

    ssize_t preadNoInt(int fd, void *buf, size_t count, off_t offset)
    {
        return wrapNoInt(pread, fd, buf, count, offset);
    }

    ssize_t pwriteNoInt(int fd, const void *buf, size_t count, off_t offset)
    {
        return wrapNoInt(pwrite, fd, buf, count, offset);
    }

    ssize_t pwriteFull(int fd, const void* buf, size_t count, off_t offset) {
        return wrapFull(pwrite, fd, const_cast<void*>(buf), count, offset);
    }

    ssize_t readvNoInt(int fd, const iovec *iov, int count)
    {
        return wrapNoInt(readv, fd, iov, count);
    }

    ssize_t writevNoInt(int fd, const iovec *iov, int count)
    {
        return wrapNoInt(writev, fd, iov, count);
    }

    ssize_t readvFull(int fd, iovec* iov, int count) {
        return wrapvFull(readv, fd, iov, count);
    }

    ssize_t writevFull(int fd, iovec* iov, int count) {
        return wrapvFull(writev, fd, iov, count);
    }

    template<class F, class... Args>
    ssize_t wrapNoInt(F f, Args... args)
    {
        ssize_t r;
        do
        {
            r = f(args...);
        } while (r == -1 && errno == EINTR);
        return r;
    }

    template <class F, class... Offset>
    ssize_t wrapFull(F f, int fd, void* buf, size_t count, Offset... offset) {
        char* b = static_cast<char*>(buf);
        ssize_t totalBytes = 0;
        ssize_t r;
        do {
            r = f(fd, b, count, offset...);
            if (r == -1) {
                if (errno == EINTR) {
                    continue;
                }
                return r;
            }

            totalBytes += r;
            b += r;
            count -= r;
            incr(r, offset...);
        } while (r != 0 && count); // 0 means EOF

        return totalBytes;
    }

    template <class F, class... Offset>
    ssize_t wrapvFull(F f, int fd, iovec* iov, int count, Offset... offset) {
        ssize_t totalBytes = 0;
        ssize_t r;
        do {
            r = f(fd, iov, std::min<int>(count, 16), offset...);
            if (r == -1) {
                if (errno == EINTR) {
                    continue;
                }
                return r;
            }

            if (r == 0) {
                break; // EOF
            }

            totalBytes += r;
            incr(r, offset...);
            while (r != 0 && count != 0) {
                if (r >= ssize_t(iov->iov_len)) {
                    r -= ssize_t(iov->iov_len);
                    ++iov;
                    --count;
                } else {
                    iov->iov_base = static_cast<char*>(iov->iov_base) + r;
                    iov->iov_len -= r;
                    r = 0;
                }
            }
        } while (count);

        return totalBytes;
    }

    template<class Container>
    bool readFile(
            int fd,
            Container &out,
            size_t num_bytes)
    {
        static_assert(
                sizeof(out[0]) == 1,
                "readFile: only containers with byte-sized elements accepted");

        size_t soFar = 0; // amount of bytes successfully read
        SCOPE_EXIT
        {
            assert(out.size() >= soFar); // resize better doesn't throw
            out.resize(soFar);
        };

        // Obtain file size:
        struct stat buf;
        if (fstat(fd, &buf) == -1)
        {
            return false;
        }
        // Some files (notably under /proc and /sys on Linux) lie about
        // their size, so treat the size advertised by fstat under advise
        // but don't rely on it. In particular, if the size is zero, we
        // should attempt to read stuff. If not zero, we'll attempt to read
        // one extra byte.
        constexpr size_t initialAlloc = 1024 * 4;
        out.resize(std::min(
                buf.st_size > 0 ? (size_t(buf.st_size) + 1) : initialAlloc, num_bytes));

        while (soFar < out.size())
        {
            const auto actual = readFull(fd, &out[soFar], out.size() - soFar);
            if (actual == -1)
            {
                return false;
            }
            soFar += actual;
            if (soFar < out.size())
            {
                // File exhausted
                break;
            }
            // Ew, allocate more memory. Use exponential growth to avoid
            // quadratic behavior. Cap size to num_bytes.
            out.resize(std::min(out.size() * 3 / 2, num_bytes));
        }

        return true;
    }

    template<class Container>
    bool readFile(
            const char *file_name,
            Container &out,
            size_t num_bytes)
    {
        assert(file_name);

        const auto fd = openNoInt(file_name, O_RDONLY | O_CLOEXEC);
        if (fd == -1)
        {
            return false;
        }

        SCOPE_EXIT
        {
            // Ignore errors when closing the file
            closeNoInt(fd);
        };

        return readFile(fd, out, num_bytes);
    }

    template<class Container>
    bool writeFile(
            const Container &data,
            const char *filename,
            int flags,
            mode_t mode)
    {
        static_assert(
                sizeof(data[0]) == 1, "writeFile works with element size equal to 1");
        int fd = open(filename, flags, mode);
        if (fd == -1)
        {
            return false;
        }
        bool ok = data.empty() ||
                  writeFull(fd, &data[0], data.size()) == static_cast<ssize_t>(data.size());
        return closeNoInt(fd) == 0 && ok;
    }

    void writeFileAtomic(
            std::string filename,
            iovec* iov,
            int count,
            mode_t permissions) {
        auto rc = writeFileAtomicNoThrow(filename, iov, count, permissions);
        if (rc != 0) {
            auto msg = std::string(__func__) + "() failed to update " + filename;
            throw std::system_error(rc, std::generic_category(), msg);
        }
    }

    int writeFileAtomicNoThrow(
            std::string filename,
            iovec* iov,
            int count,
            mode_t permissions) {
        // We write the data to a temporary file name first, then atomically rename
        // it into place.  This ensures that the file contents will always be valid,
        // even if we crash or are killed partway through writing out data.
        //
        // Create a buffer that will contain two things:
        // - A nul-terminated version of the filename
        // - The temporary file name
        std::vector<char> pathBuffer;
        // Note that we have to explicitly pass in the size here to make
        // sure the nul byte gets included in the data.
        const std::string suffix(".XXXXXX\0");
        pathBuffer.resize((2 * filename.size()) + 1 + suffix.size());
        // Copy in the filename and then a nul terminator
        memcpy(pathBuffer.data(), filename.data(), filename.size());
        pathBuffer[filename.size()] = '\0';
        const char* const filenameCStr = pathBuffer.data();
        // Now prepare the temporary path template
        char* const tempPath = pathBuffer.data() + filename.size() + 1;
        memcpy(tempPath, filename.data(), filename.size());
        memcpy(tempPath + filename.size(), suffix.data(), suffix.size());

        auto tmpFD = mkstemp(tempPath);
        if (tmpFD == -1) {
            return errno;
        }
        bool success = false;
        SCOPE_EXIT {
                       if (tmpFD != -1) {
                           close(tmpFD);
                       }
                       if (!success) {
                           unlink(tempPath);
                       }
                   };

        auto rc = writevFull(tmpFD, iov, count);
        if (rc == -1) {
            return errno;
        }

        rc = fchmod(tmpFD, permissions);
        if (rc == -1) {
            return errno;
        }

        // Close the file before renaming to make sure all data has
        // been successfully written.
        rc = close(tmpFD);
        tmpFD = -1;
        if (rc == -1) {
            return errno;
        }

        rc = rename(tempPath, filenameCStr);
        if (rc == -1) {
            return errno;
        }
        success = true;
        return 0;
    }
}
