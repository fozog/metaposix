cmake_minimum_required(VERSION 3.16)
project(metaposix C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are not supported. Configure with "
        "\"cmake -S . -B build\" so generated files stay out of the source tree.")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(MSVC)
    message(FATAL_ERROR
        "metaposix currently requires a POSIX-compatible C environment and "
        "GNU-style #include_next support; native MSVC is not supported. "
        "Use a POSIX environment or provide a separate Windows backend.")
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang|ARMClang")
    add_compile_options(-Wall -Wextra)
endif()

include(CheckTypeSize)
check_type_size("wchar_t" MP_WCHAR_T_SIZE LANGUAGE C)
if(MP_WCHAR_T_SIZE)
    message(STATUS
        "Native wchar_t is ${MP_WCHAR_T_SIZE} bytes; its width/encoding are "
        "platform-defined. Use char32_t for fixed-width Unicode code points.")
endif()

# Runtime portability layer: interposed headers and their shim implementations.
add_library(metaposix STATIC
    src/mp_sem.c
    src/mp_barrier.c
    src/mp_ppoll.c
    src/mp_string.c
    src/mp_uchar.c
)
add_library(metaposix::metaposix ALIAS metaposix)
target_include_directories(metaposix BEFORE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)
target_link_libraries(metaposix PUBLIC Threads::Threads)

# Older glibc keeps POSIX AIO in librt.
find_library(RT_LIBRARY rt)
if(RT_LIBRARY)
    target_link_libraries(metaposix PUBLIC ${RT_LIBRARY})
endif()

# Validation program: deliberately separate from the runtime library.
add_executable(mp_tests
    tests/main.c
    tests/int_string.c
    tests/thread_sync.c
    tests/file_fs.c
    tests/net.c
    tests/aio.c
)
target_link_libraries(mp_tests PRIVATE metaposix::metaposix)

# Keep the portability shims on the C11 baseline, while compiling the sample
# in C23 mode when this compiler supports 128-bit _BitInt.
include(CheckCSourceCompiles)
set(_mp_saved_required_flags "${CMAKE_REQUIRED_FLAGS}")
set(_mp_saved_c_standard "${CMAKE_C_STANDARD}")
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang|ARMClang")
    unset(CMAKE_C_STANDARD)
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c2x")
    check_c_source_compiles("
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202000L
#error C23 mode is required
#endif
#if !defined(__BITINT_MAXWIDTH__) || __BITINT_MAXWIDTH__ < 128
#error 128-bit _BitInt is required
#endif
int main(void) {
    unsigned _BitInt(128) value = 1;
    return (int)(value + 1);
}
" METAPOSIX_HAS_C23_BITINT128)
    set(CMAKE_REQUIRED_FLAGS "${_mp_saved_required_flags}")
    set(CMAKE_C_STANDARD "${_mp_saved_c_standard}")
endif()
unset(_mp_saved_required_flags)
unset(_mp_saved_c_standard)
if(METAPOSIX_HAS_C23_BITINT128)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang|ARMClang")
        target_compile_options(mp_tests PRIVATE -std=c2x)
    endif()
else()
    message(STATUS "Compiler lacks C23 128-bit _BitInt; sample will skip that test")
endif()

enable_testing()
add_test(NAME mp_tests COMMAND mp_tests)
