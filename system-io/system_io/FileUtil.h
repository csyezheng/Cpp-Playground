#ifndef SYSTEM_IO_FILEUTIL_H
#define SYSTEM_IO_FILEUTIL_H

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/fcntl.h>
#include <cassert>
#include <string>
#include <limits>

/*
 * Convenience wrappers around some commonly used system calls.
 * The *NoInt wrappers retry on EINTR.
 * The *Full wrappers retry on EINTR and also loop until all data is written.
 * Note that *Full wrappers weaken the thread semantics of underlying system calls.
 */

namespace sysio
{

    int openNoInt(const char *name, int flags, mode_t mode = 0666);

    int closeNoInt(int fd);

    int dupNoInt(int fd);

    int dup2NoInt(int oldFd, int newFd);

    /*
     * flock should not be interrupted
     */
    int flockNoInt(int fd, int operation);

    /*
     * Read from a file or socket
     */
    ssize_t readNoInt(int fd, void *buf, size_t n);

    ssize_t writeNoInt(int fd, const void *buf, size_t n);

    ssize_t readFull(int fd, void* buf, size_t n);

    ssize_t writeFull(int fd, const void* buf, size_t n);

    /*
     * Read from a file or socket without file pointer change
     */
    ssize_t preadNoInt(int fd, void *buf, size_t n, off_t offset);

    ssize_t pwriteNoInt(int fd, const void *buf, size_t n, off_t offset);

    ssize_t preadFull(int fd, void* buf, size_t n, off_t offset);

    ssize_t pwriteFull(int fd, const void* buf, size_t n, off_t offset);

    /*
     * read or write a vector
     * Read data on a file or socket and store in a set of buffers described by IOVEC.
     * data are put in IOVEC instead of a contiguous buffer.
     */
    ssize_t readvNoInt(int fd, const iovec *iov, int count);

    ssize_t writevNoInt(int fd, const iovec *iov, int count);

    ssize_t readvFull(int fd, iovec* iov, int count);

    ssize_t writevFull(int fd, iovec* iov, int count);

    /*
     * Wrap call to f(args) in loop to retry on EINTR
     */
    template<class F, class... Args>
    ssize_t wrapNoInt(F f, Args... args);

    template <class F, class... Offset>
    ssize_t wrapvFull(F f, int fd, iovec* iov, int count, Offset... offset);

    template <class F, class... Offset>
    ssize_t wrapFull(F f, int fd, void* buf, size_t count, Offset... offset);

    /*
     * Read entire file (if num_bytes is defaulted) or no more than
     * num_bytes (otherwise) into container *out. The container is assumed
     * to be contiguous, with element size equal to 1, and offer size(),
     * reserve(), and random access (e.g. std::vector<char>, std::string).
     *
     * Returns: true on success or false on failure. In the latter case
     * errno will be set appropriately by the failing system primitive.
     */
    template<class Container>
    bool readFile(
            int fd,
            Container &out,
            size_t num_bytes = std::numeric_limits<size_t>::max());

    /*
     * Same as above, but takes in a file name instead of fd
     */
    template <class Container>
    bool readFile(
            const char* file_name,
            Container& out,
            size_t num_bytes = std::numeric_limits<size_t>::max());

    /*
     * Writes container to file. The container is assumed to be
     * contiguous, with element size equal to 1, and offering STL-like
     * methods empty(), size(), and indexed access
     * (e.g. std::vector<char>, std::string).
     *
     * "flags" dictates the open flags to use. Default is to create file
     * if it doesn't exist and truncate it.
     *
     * Returns: true on success or false on failure. In the latter case
     * errno will be set appropriately by the failing system primitive.
     *
     * Note that this function may leave the file in a partially written state on
     * failure.  Use writeFileAtomic() if you want to ensure that the existing file
     * state will be unchanged on error.
     */
    template <class Container>
    bool writeFile(
            const Container& data,
            const char* filename,
            int flags = O_WRONLY | O_CREAT | O_TRUNC,
            mode_t mode = 0666);

    /*
     * Write file contents "atomically".
     *
     * This writes the data to a temporary file in the destination directory, and
     * then renames it to the specified path.  This guarantees that the specified
     * file will be replaced the the specified contents on success, or will not be
     * modified on failure.
     *
     * Note that on platforms that do not provide atomic filesystem rename
     * functionality (e.g., Windows) this behavior may not be truly atomic.
     */
    void writeFileAtomic(
            std::string filename,
            iovec* iov,
            int count,
            mode_t permissions = 0644);

    /*
     * A version of writeFileAtomic() that returns an errno value instead of
     * throwing on error.
     *
     * Returns 0 on success or an errno value on error.
     */
    int writeFileAtomicNoThrow(
            std::string filename,
            iovec* iov,
            int count,
            mode_t permissions = 0644);
}

#endif //SYSTEM_IO_FILEUTIL_H
