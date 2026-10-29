/*
 * metaposix — platform detection and capability map.
 *
 * MP_HAS_* macros answer, per platform, "does the native environment
 * provide a *working* implementation of this POSIX.1-2024 facility?"
 * When the answer is 0, metaposix either supplies a shim (see the
 * interposed headers in include/ and the sources in src/) or the macro
 * simply reports the hole honestly so application code can adapt.
 *
 * In a full implementation these answers would come from build-time
 * probes (configs/<platform>.h); this prototype uses compile-time
 * knowledge of the supported platforms.
 */
#ifndef METAPOSIX_MP_FEATURES_H
#define METAPOSIX_MP_FEATURES_H

#include <metaposix/mp_compiler.h>

/* ---- OS identification -------------------------------------------------- */
#if defined(__APPLE__)
#  define MP_OS_MACOS 1
#elif defined(__linux__)
#  define MP_OS_LINUX 1
#elif defined(__FreeBSD__)
#  define MP_OS_FREEBSD 1
#elif defined(__NetBSD__)
#  define MP_OS_NETBSD 1
#elif defined(__OpenBSD__)
#  define MP_OS_OPENBSD 1
#elif defined(__ZEPHYR__)
#  define MP_OS_ZEPHYR 1
#else
#  define MP_OS_UNKNOWN 1
#endif

#ifndef MP_OS_MACOS
#  define MP_OS_MACOS 0
#endif
#ifndef MP_OS_LINUX
#  define MP_OS_LINUX 0
#endif
#ifndef MP_OS_FREEBSD
#  define MP_OS_FREEBSD 0
#endif
#ifndef MP_OS_NETBSD
#  define MP_OS_NETBSD 0
#endif
#ifndef MP_OS_OPENBSD
#  define MP_OS_OPENBSD 0
#endif
#ifndef MP_OS_ZEPHYR
#  define MP_OS_ZEPHYR 0
#endif

#define MP_OS_BSD (MP_OS_FREEBSD || MP_OS_NETBSD || MP_OS_OPENBSD)

/* ---- Capability map ------------------------------------------------------ */

/* Unnamed semaphores: macOS's sem_init() returns -1/ENOSYS.               */
#if MP_OS_MACOS
#  define MP_HAS_WORKING_SEM_INIT 0
#else
#  define MP_HAS_WORKING_SEM_INIT 1
#endif

/* sem_timedwait(): absent on macOS.                                        */
#if MP_OS_MACOS
#  define MP_HAS_SEM_TIMEDWAIT 0
#else
#  define MP_HAS_SEM_TIMEDWAIT 1
#endif

/* pthread barriers (option _POSIX_BARRIERS): absent on macOS.              */
#if MP_OS_MACOS
#  define MP_HAS_PTHREAD_BARRIER 0
#else
#  define MP_HAS_PTHREAD_BARRIER 1
#endif

/* ppoll(): standardized by POSIX.1-2024; Linux/FreeBSD have it,            */
/* macOS/NetBSD do not (NetBSD calls it pollts).                            */
#if MP_OS_LINUX || MP_OS_FREEBSD || MP_OS_OPENBSD
#  define MP_HAS_PPOLL 1
#else
#  define MP_HAS_PPOLL 0
#endif

/* <uchar.h> (char16_t/char32_t conversions): Apple SDKs lack it         */
/* entirely, so metaposix interposes its own (include/uchar.h), which     */
/* forwards to the native header when one exists and shims mbrtoc32/      */
/* c32rtomb otherwise.  MP_UCHAR_SHIMMED (defined by that header) tells   */
/* you which path you are on.                                             */

/* POSIX AIO (<aio.h>): present on macOS/Linux/BSD; absent on some RTOS.    */
#if MP_HAS_INCLUDE(<aio.h>) && !MP_OS_ZEPHYR
#  define MP_HAS_POSIX_AIO 1
#else
#  define MP_HAS_POSIX_AIO 0
#endif

/* strlcpy/strlcat: BSD/macOS always; glibc only since 2.38.                 */
/* memmem: standardized by POSIX.1-2024; BSD/macOS/glibc have it.            */
/* These depend on the libc actually included, so they are finalized in the  */
/* interposed <string.h> once the real header (and __GLIBC__) is visible.    */

#endif /* METAPOSIX_MP_FEATURES_H */
