#ifndef SYSTEM_IO_LIKE_H
#define SYSTEM_IO_LIKE_H

/*
 * use __builtin_expect to provide the compiler with branch prediction information
 */

#undef LIKELY
#undef UNLIKELY

#if defined(__GNUC__)
#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif


#endif //SYSTEM_IO_LIKE_H
