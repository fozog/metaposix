/*
 * metaposix shim: strlcpy()/strlcat()/memmem(), promoted to the
 * standard by POSIX.1-2024 but missing from older glibc.
 *
 * On platforms that already provide them (macOS/BSD, glibc >= 2.38)
 * the interposed <string.h> keeps the native versions and this file
 * compiles to nothing.
 */
#include "metaposix.h"

#include <string.h>

#if !MP_HAS_STRLCPY

size_t mp_strlcpy(char *dst, const char *src, size_t dstsize)
{
    size_t srclen = strlen(src);
    if (dstsize != 0) {
        size_t n = srclen < dstsize - 1 ? srclen : dstsize - 1;
        memcpy(dst, src, n);
        dst[n] = '\0';
    }
    return srclen;
}

size_t mp_strlcat(char *dst, const char *src, size_t dstsize)
{
    size_t dstlen = strnlen(dst, dstsize);
    size_t srclen = strlen(src);
    if (dstlen == dstsize)
        return dstsize + srclen;
    size_t space = dstsize - dstlen - 1;
    size_t n = srclen < space ? srclen : space;
    memcpy(dst + dstlen, src, n);
    dst[dstlen + n] = '\0';
    return dstlen + srclen;
}

#endif /* !MP_HAS_STRLCPY */

#if !MP_HAS_MEMMEM

void *mp_memmem(const void *haystack, size_t haystacklen,
                const void *needle, size_t needlelen)
{
    if (needlelen == 0)
        return (void *)haystack;
    if (haystacklen < needlelen)
        return NULL;
    const char *h = (const char *)haystack;
    const char *n = (const char *)needle;
    for (size_t i = 0; i + needlelen <= haystacklen; i++) {
        if (h[i] == n[0] && memcmp(h + i, n, needlelen) == 0)
            return (void *)(h + i);
    }
    return NULL;
}

#endif /* !MP_HAS_MEMMEM */
