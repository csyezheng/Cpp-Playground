#ifndef SYSTEM_IO_EXCEPTION_H
#define SYSTEM_IO_EXCEPTION_H

#include <string>
#include <system_error>
#include "system_io/Likely.h"

namespace sysio
{
    inline std::system_error makeSystemErrorExplicit(int err, const std::string msg)
    {
        return std::system_error(err, std::generic_category(), msg);
    }

    template<typename Ex>
    [[noreturn]] void throw_exception(Ex &&ex)
    {
#ifndef NO_EXCEPTIONS
        throw static_cast<Ex &&>(ex);
#else
        (void) ex;
        std::terminate();
#endif
    }

    // Helper to throw std::system_error
    [[noreturn]] inline void throwSystemErrorExplicit(int err, const std::string &msg)
    {
        throw_exception(makeSystemErrorExplicit(err, msg));
    }

// Helper to throw std::system_error from errno and components of a string
    template<class... Args>
    [[noreturn]] void throwSystemError(Args &&... args)
    {
        throwSystemErrorExplicit(errno, std::forward<Args>(args)...);
    }

    // Check the return code from a fopen-style function (returns a non-nullptr
    // FILE* on success, nullptr on error, sets errno).  Works with fopen, fdopen,
    // freopen, tmpfile, etc.
    template<class... Args>
    void checkFopenError(FILE *fp, Args &&... args)
    {
        if (UNLIKELY(!fp))
        {
            throwSystemError(std::forward<Args>(args)...);
        }
    }

    // Check a traditional Unix return code (-1 and sets errno on error), throw
    // on error.
    template<class... Args>
    void checkUnixError(ssize_t ret, Args &&... args)
    {
        if (UNLIKELY(ret == -1))
        {
            throwSystemError(std::forward<Args>(args)...);
        }
    }
}

#endif //SYSTEM_IO_EXCEPTION_H
