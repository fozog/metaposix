#define _POSIX_C_SOURCE 200809L
#if defined(__APPLE__)
#define _DARWIN_C_SOURCE
#endif

#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <uchar.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202000L && \
    defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 128
#define MP_SAMPLE_HAS_BITINT128 1
typedef unsigned _BitInt(128) mp_uint128_t;
typedef _BitInt(128) mp_int128_t;

static void
uint128_to_decimal(mp_uint128_t value, char *out)
{
    char reversed[39];
    size_t digits = 0;

    do {
        reversed[digits++] = (char)('0' + value % 10);
        value /= 10;
    } while (value != 0);

    for (size_t i = 0; i < digits; i++)
        out[i] = reversed[digits - i - 1];
    out[digits] = '\0';
}

static void
int128_to_decimal(mp_int128_t value, char *out)
{
    int negative = value < 0;
    mp_uint128_t magnitude = (mp_uint128_t)value;

    if (negative) {
        magnitude = (mp_uint128_t)0 - magnitude;
        out[0] = '-';
    }
    uint128_to_decimal(magnitude, out + negative);
}

static int
test_c23_integer_128(void)
{
    const mp_uint128_t one = 1;
    const mp_uint128_t base = (one << 64) + 12345;
    const mp_uint128_t other = (one << 64) + 67890;
    const mp_uint128_t sum = base + other;
    const mp_uint128_t product = base * 17;
    const mp_uint128_t unsigned_max = ((one << 127) - 1) * 2 + 1;
    const mp_int128_t signed_max = (mp_int128_t)((one << 127) - 1);
    const mp_int128_t signed_min = -signed_max - 1;
    char base_text[40];
    char sum_text[40];
    char product_text[40];
    char unsigned_max_text[40];
    char signed_max_text[42];
    char signed_min_text[42];

    uint128_to_decimal(base, base_text);
    uint128_to_decimal(sum, sum_text);
    uint128_to_decimal(product, product_text);
    uint128_to_decimal(unsigned_max, unsigned_max_text);
    int128_to_decimal(signed_max, signed_max_text);
    int128_to_decimal(signed_min, signed_min_text);

    if (product / 17 != base || product % 17 != 0 ||
        sum - base != other ||
        strcmp(base_text, "18446744073709563961") != 0 ||
        strcmp(unsigned_max_text,
               "340282366920938463463374607431768211455") != 0 ||
        strcmp(signed_max_text,
               "170141183460469231731687303715884105727") != 0 ||
        strcmp(signed_min_text,
               "-170141183460469231731687303715884105728") != 0)
        return fprintf(stderr, "FAIL C23 128-bit integer arithmetic/printing\n"),
               -1;

    printf("PASS C23 128-bit arithmetic: a=%s sum=%s product=%s\n",
           base_text, sum_text, product_text);
    printf("PASS C23 128-bit decimal limits: unsigned max=%s signed min=%s\n",
           unsigned_max_text, signed_min_text);
    return 0;
}
#else
#define MP_SAMPLE_HAS_BITINT128 0
#endif

int
mp_test_int_string(void)
{
    const intmax_t signed_max = INTMAX_MAX;
    const uintmax_t unsigned_max = UINTMAX_MAX;
    wchar_t wide_copy[16];
    const wchar_t wide_text[] = L"wide POSIX";
    wchar_t *wide_end = NULL;
    char narrow_copy[32];
    char *narrow_end = NULL;
    intmax_t parsed;
    size_t length;

    if (signed_max / 3 <= 0 || unsigned_max / 3 <= 0 ||
        (signed_max - 100) + 99 != signed_max - 1 ||
        unsigned_max + UINTMAX_C(1) != 0)
        return fprintf(stderr, "FAIL wide integers: arithmetic\n"), -1;

    parsed = strtoimax("9223372036854775807", &narrow_end, 10);
    if (parsed != INTMAX_MAX || narrow_end == NULL || *narrow_end != '\0')
        return fprintf(stderr, "FAIL wide integers: strtoimax\n"), -1;
    parsed = wcstoimax(L"9223372036854775807", &wide_end, 10);
    if (parsed != INTMAX_MAX || wide_end == NULL || *wide_end != L'\0')
        return fprintf(stderr, "FAIL wide integers: wcstoimax\n"), -1;

    length = strlcpy(narrow_copy, "char string", sizeof narrow_copy);
    if (length != strlen("char string") ||
        strlcat(narrow_copy, " functions", sizeof narrow_copy) !=
            strlen("char string functions") ||
        strcmp(narrow_copy, "char string functions") != 0 ||
        memmem(narrow_copy, strlen(narrow_copy), "string", 6) !=
            narrow_copy + 5)
        return fprintf(stderr, "FAIL narrow strings\n"), -1;

    if (wcscpy(wide_copy, wide_text) != wide_copy ||
        wcscmp(wide_copy, wide_text) != 0 ||
        wcslen(wide_copy) != 10)
        return fprintf(stderr, "FAIL wide strings\n"), -1;

    puts("PASS wide integer arithmetic and narrow/wide string functions");

#if MP_SAMPLE_HAS_BITINT128
    if (test_c23_integer_128() != 0)
        return -1;
#else
    puts("SKIP C23 128-bit integers: compiler mode or _BitInt width unavailable");
#endif

    if (setlocale(LC_CTYPE, "") == NULL) {
        puts("SKIP UTF-8 conversion: locale could not be selected");
        return 0;
    }

    {
        const char utf8[] = "\xC3\xA9";
        mbstate_t state = { 0 };
        char32_t codepoint = 0;
        size_t consumed = mbrtoc32(&codepoint, utf8, sizeof utf8, &state);

        if (consumed == (size_t)-1 || consumed == (size_t)-2 ||
            codepoint != U'\u00E9') {
            puts("SKIP UTF-8 conversion: active locale does not decode U+00E9");
            return 0;
        }

        state = (mbstate_t){ 0 };
        char encoded[MB_LEN_MAX];
        size_t produced = c32rtomb(encoded, codepoint, &state);
        if (produced != 2 || memcmp(encoded, utf8, 2) != 0)
            return fprintf(stderr, "FAIL UTF-8 round trip\n"), -1;
    }

    puts("PASS UTF-8 / char32_t conversion");
    return 0;
}
