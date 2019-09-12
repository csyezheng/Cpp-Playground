#include "system_io/File.h"

#include <iostream>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <system_error>

#include <bsd/string.h>
#include <glog/logging.h>
#include "system_io/ScopeGuard.h"

#include "system_io/Exception.h"
//#include "ScopeGuard.h"
#include "system_io/FileUtil.h"


namespace sysio
{

    File::File() noexcept : fd_(-1), ownsFd_(false)
    {}

    File::File(int fd, bool ownsFd) noexcept : fd_(fd), ownsFd_(ownsFd)
    {
        CHECK_GE(fd, -1) << "fd must be -1 or non-negative";
        CHECK(fd != -1 || !ownsFd) << "cannot own -1";
    }

    File::File(const char *name, int flags, mode_t mode)
            : fd_(::open(name, flags, mode)), ownsFd_(false)
    {
        if (fd_ == -1)
        {
            std::stringstream code;
            // UNIX file permissions are conventionally expressed in octal
            // while it is in decimal in C language
            code << std::oct << mode;
            // add leading zero to string because it represent octal number
            std::string perms = std::string("0").append(code.str());
            std::stringstream msg;
            msg << "open(" << name << ", " << flags << ", " << perms << " failed";
            throwSystemError(msg.str());
        }
        ownsFd_ = true;
    }

    File::File(const std::string &name, int flags, mode_t mode)
            : File(name.c_str(), flags, mode)
    {}

    File::File(File&& other) noexcept : fd_(other.fd_), ownsFd_(other.ownsFd_) {
        other.release();
    }

    File& File::operator=(File&& other) noexcept {
        closeNoThrow();
        swap(other);
        return *this;
    }

    File::~File()
    {
        auto fd = fd_;
        if (!closeNoThrow())
        { // ignore most errors
            DCHECK_NE(errno, EBADF)
                << "closing fd " << fd << ", it may already "
                << "have been closed. Another time, this might close the wrong FD.";
        }
    }

    File File::temporary()
    {
        // make a temp file with tmpfile(), dup the fd, then return it in a File.
        std::FILE *tmpf = std::tmpfile();
        checkFopenError(tmpf, "tmpfile() failed");
        SCOPE_EXIT
        {
            fclose(tmpf);
        };
        int fd = ::dup(fileno(tmpf));
        checkUnixError(fd, "dup() failed");

        return File(fd, true);
    }

    int File::fd() const
    {
        return fd_;
    }

    File::operator bool() const {
        return fd_ != -1;
    }

    File File::dup() const
    {
        if (fd_ != -1) {
            int fd = ::dup(fd_);
            checkUnixError(fd, "dup() failed");

            return File(fd, true);
        }
        return File();
    }

    void File::close()
    {
        if (!closeNoThrow())
        {
            throwSystemError("close() failed");
        }
    }

    bool File::closeNoThrow()
    {
        int r = ownsFd_ ? ::close(fd_) : 0;
        release();
        return r == 0;
    }

    int File::release() noexcept
    {
        int released = fd_;
        fd_ = -1;
        ownsFd_ = false;
        return released;
    }

    void File::swap(File& other) noexcept {
        using std::swap;
        swap(fd_, other.fd_);
        swap(ownsFd_, other.ownsFd_);
    }

    void File::lock() {
        doLock(LOCK_EX);
    }

    bool File::try_lock() {
        return doTryLock(LOCK_EX);
    }

    void File::unlock() {
        checkUnixError(flockNoInt(fd_, LOCK_UN), "flock() failed (unlock)");
    }

    void File::lock_shared() {
        doLock(LOCK_SH);
    }

    bool File::try_lock_shared() {
        return doTryLock(LOCK_SH);
    }

    void File::unlock_shared() {
        unlock();
    }

    void File::doLock(int op) {
        checkUnixError(flockNoInt(fd_, op), "flock() failed (lock)");
    }

    bool File::doTryLock(int op) {
        int r = flockNoInt(fd_, op | LOCK_NB);
        // flock returns EWOULDBLOCK if already locked
        if (r == -1 && errno == EWOULDBLOCK) {
            return false;
        }
        checkUnixError(r, "flock() failed (try_lock)");
        return true;
    }


    void swap(File& a, File& b) noexcept {
        a.swap(b);
    }
}