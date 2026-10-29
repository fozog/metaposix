/*
 * metaposix — compiler abstraction layer.
 *
 * Normalizes attributes, packing, alignment and builtins across the
 * supported compiler baseline: GCC >= 9, Clang/AppleClang >= 10,
 * Arm Compiler 6 (armclang).  All three are GNU-C compatible, which is
 * what makes a single mapping possible.
 */
#ifndef METAPOSIX_MP_COMPILER_H
#define METAPOSIX_MP_COMPILER_H

/* ---- Compiler identification ------------------------------------------ */
#if defined(__ARMCOMPILER_VERSION)          /* Arm Compiler 6 (armclang)   */
#  define MP_CC_ARMCLANG 1
#elif defined(__clang__)                    /* Clang / AppleClang          */
#  define MP_CC_CLANG 1
#elif defined(__GNUC__)                     /* GCC                         */
#  define MP_CC_GCC 1
#else
#  error "metaposix: unsupported compiler (need GCC, Clang or Arm Compiler 6)"
#endif

/* ---- Language baseline ------------------------------------------------- */
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#  error "metaposix: C11 or later is required"
#endif

/* ---- Alignment (standard C11 mechanisms, wrapped for symmetry) --------- */
#include <stdalign.h>
#define MP_ALIGNAS(n)   _Alignas(n)
#define MP_ALIGNOF(t)   _Alignof(t)
#define MP_ALIGNED(n)   __attribute__((aligned(n)))   /* for types/objects */

/* ---- Packing -----------------------------------------------------------
 * There is no standard packing mechanism.  All supported compilers share
 * the GNU attribute.  NEVER apply this to structs that cross the platform
 * libc/kernel ABI boundary (struct stat, sockaddr_*, ...): those must keep
 * the platform's own layout.
 */
#define MP_PACKED       __attribute__((packed))

/* ---- Common attributes -------------------------------------------------- */
#define MP_NORETURN     _Noreturn
#define MP_UNUSED       __attribute__((unused))
#define MP_USED         __attribute__((used))
#define MP_WEAK         __attribute__((weak))
#define MP_ALWAYS_INLINE __attribute__((always_inline)) inline
#define MP_NOINLINE     __attribute__((noinline))
#define MP_DEPRECATED(msg) __attribute__((deprecated(msg)))
#define MP_PRINTF(fmt_idx, arg_idx) __attribute__((format(printf, fmt_idx, arg_idx)))
#define MP_WARN_UNUSED  __attribute__((warn_unused_result))

/* ---- Branch hints / builtins ------------------------------------------- */
#define MP_LIKELY(x)    __builtin_expect(!!(x), 1)
#define MP_UNLIKELY(x)  __builtin_expect(!!(x), 0)

/* ---- Feature-test helper ------------------------------------------------ */
#if defined(__has_include)
#  define MP_HAS_INCLUDE(hdr) __has_include(hdr)
#else
#  define MP_HAS_INCLUDE(hdr) 0
#endif

#endif /* METAPOSIX_MP_COMPILER_H */
