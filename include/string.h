/*
 * metaposix interposed <string.h>
 *
 * POSIX.1-2024 promoted several long-standing extensions into the
 * standard: strlcpy(), strlcat(), memmem().  Older glibc (< 2.38) lacks
 * strlcpy/strlcat entirely, and glibc hides memmem unless _GNU_SOURCE
 * is defined.  This wrapper finalizes detection *after* the real header
 * is visible (so __GLIBC__ is available) and declares polyfills from
 * src/mp_string.c where needed.
 */
#ifndef METAPOSIX_STRING_H
#define METAPOSIX_STRING_H

#include_next <string.h>

#include <metaposix/mp_features.h>

/* ---- strlcpy / strlcat --------------------------------------------------- */
#if MP_OS_MACOS || MP_OS_BSD
#  define MP_HAS_STRLCPY 1
#elif defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 38))
#  define MP_HAS_STRLCPY 1
#else
#  define MP_HAS_STRLCPY 0
#endif

#if !MP_HAS_STRLCPY
size_t mp_strlcpy(char *dst, const char *src, size_t dstsize);
size_t mp_strlcat(char *dst, const char *src, size_t dstsize);
#  define strlcpy mp_strlcpy
#  define strlcat mp_strlcat
#endif

/* ---- memmem --------------------------------------------------------------- */
#if MP_OS_MACOS || MP_OS_BSD
#  define MP_HAS_MEMMEM 1
#elif defined(__GLIBC__) && defined(_GNU_SOURCE)
#  define MP_HAS_MEMMEM 1
#else
#  define MP_HAS_MEMMEM 0
#endif

#if !MP_HAS_MEMMEM
void *mp_memmem(const void *haystack, size_t haystacklen,
                const void *needle, size_t needlelen);
#  define memmem mp_memmem
#endif

#endif /* METAPOSIX_STRING_H */
