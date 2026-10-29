#include <stdio.h>
#include <stdlib.h>

#include "test.h"

int
main(void)
{
    if (mp_test_int_string() != 0 ||
        mp_test_thread_sync() != 0 ||
        mp_test_file_fs() != 0 ||
        mp_test_net() != 0 ||
        mp_test_aio() != 0)
        return EXIT_FAILURE;

    puts("All available metaposix sample tests passed.");
    return EXIT_SUCCESS;
}
