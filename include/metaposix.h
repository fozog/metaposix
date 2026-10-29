#ifndef METAPOSIX_H
#define METAPOSIX_H

/* Must precede system headers: feature-test macros affect their declarations. */
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE < 200809L
#error "metaposix requires _POSIX_C_SOURCE >= 200809L"
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE
#endif

#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <metaposix/mp_features.h>

#endif
