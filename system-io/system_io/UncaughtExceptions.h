#ifndef SYSTEM_IO_UNCAUGHTEXCEPTIONS_H
#define SYSTEM_IO_UNCAUGHTEXCEPTIONS_H

#include <exception>

#if !defined(FORCE_EXCEPTION_COUNT_USE_STD) && defined(__GNUG__)
#define EXCEPTION_COUNT_USE_CXA_GET_GLOBALS
namespace __cxxabiv1 {
// forward declaration (originally defined in unwind-cxx.h from from libstdc++)
    struct __cxa_eh_globals;
// declared in cxxabi.h from libstdc++-v3
    extern "C" __cxa_eh_globals* __cxa_get_globals() noexcept;
} // namespace __cxxabiv1
#elif defined(FORCE_EXCEPTION_COUNT_USE_STD) || defined(_MSC_VER)
#define EXCEPTION_COUNT_USE_STD
#else
// Raise an error when trying to use this on unsupported platforms.
#error "Unsupported platform, don't include this header."
#endif

namespace sysio {

/**
 * Returns the number of uncaught exceptions.
 */
    inline int uncaught_exceptions() noexcept {
#if defined(EXCEPTION_COUNT_USE_CXA_GET_GLOBALS)
        // __cxa_get_globals returns a __cxa_eh_globals* (defined in unwind-cxx.h).
        // The offset below returns __cxa_eh_globals::uncaughtExceptions.
        return *(reinterpret_cast<unsigned int*>(
                static_cast<char*>(static_cast<void*>(__cxxabiv1::__cxa_get_globals())) +
                sizeof(void*)));
#elif defined(EXCEPTION_COUNT_USE_GETPTD)
        // _getptd() returns a _tiddata* (defined in mtdll.h).
  // The offset below returns _tiddata::_ProcessingThrow.
  return *(reinterpret_cast<int*>(
      static_cast<char*>(static_cast<void*>(_getptd())) + sizeof(void*) * 28 +
      0x4 * 8));
#elif defined(EXCEPTION_COUNT_USE_STD)
  return std::uncaught_exceptions();
#endif
    }

}

#endif //SYSTEM_IO_UNCAUGHTEXCEPTIONS_H
