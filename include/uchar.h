/*
 * metaposix interposed <uchar.h>
 *
 * Apple SDKs ship no <uchar.h> at all (a plain C11 conformance hole).
 * Where the platform has one we forward to it; where it does not,
 * metaposix supplies char16_t/char32_t and mbrtoc32()/c32rtomb()
 * built on mbrtowc()/wcrtomb(), which is exact on every platform
 * where wchar_t is UTF-32 (macOS, Linux, the BSDs).
 *
 * mbrtoc16()/c16rtomb() (UTF-16 surrogate handling) are deliberately
 * not provided by this prototype.
 */
#ifndef METAPOSIX_UCHAR_H
#define METAPOSIX_UCHAR_H

#include <metaposix/mp_features.h>

#if defined(__has_include_next)
#  if __has_include_next(<uchar.h>)
#    define MP_UCHAR_SHIMMED 0
#    include_next <uchar.h>
#  endif
#endif

#ifndef MP_UCHAR_SHIMMED
#define MP_UCHAR_SHIMMED 1

#include <stddef.h>
#include <stdint.h>
#include <wchar.h>      /* mbstate_t */

#ifndef __STDC_UTF_32__
#define __STDC_UTF_32__ 1
#endif

typedef uint_least16_t char16_t;
typedef uint_least32_t char32_t;

size_t mp_mbrtoc32(char32_t *pc32, const char *s, size_t n, mbstate_t *ps);
size_t mp_c32rtomb(char *s, char32_t c32, mbstate_t *ps);

#define mbrtoc32 mp_mbrtoc32
#define c32rtomb mp_c32rtomb

#endif /* shim */

#endif /* METAPOSIX_UCHAR_H */
