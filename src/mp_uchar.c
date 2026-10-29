/*
 * metaposix shim: mbrtoc32()/c32rtomb() for platforms without
 * <uchar.h> (Apple SDKs).
 *
 * Valid wherever wchar_t holds UTF-32 scalar values, which is the case
 * on macOS, Linux and the BSDs (NOT on Windows, where wchar_t is
 * UTF-16 — that port would need real surrogate handling).
 */
#include <uchar.h>

#if MP_UCHAR_SHIMMED

#include <errno.h>
#include <wchar.h>

size_t mp_mbrtoc32(char32_t *pc32, const char *s, size_t n, mbstate_t *ps)
{
    static mbstate_t internal;
    if (ps == NULL)
        ps = &internal;
    if (s == NULL)                       /* equivalent to mbrtoc32(NULL,"",1,ps) */
        return mbrtowc(NULL, NULL, 0, ps);

    wchar_t wc;
    size_t r = mbrtowc(&wc, s, n, ps);
    if (r != (size_t)-1 && r != (size_t)-2 && pc32 != NULL)
        *pc32 = (char32_t)wc;
    return r;
}

size_t mp_c32rtomb(char *s, char32_t c32, mbstate_t *ps)
{
    static mbstate_t internal;
    if (ps == NULL)
        ps = &internal;
    return wcrtomb(s, (wchar_t)c32, ps);
}

#endif /* MP_UCHAR_SHIMMED */
