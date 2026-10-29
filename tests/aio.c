#define _POSIX_C_SOURCE 200809L
#if defined(__APPLE__)
#define _DARWIN_C_SOURCE
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <metaposix/mp_features.h>
#if MP_HAS_POSIX_AIO
#include <aio.h>
#endif

#include "test.h"

int
mp_test_aio(void)
{
#if MP_HAS_POSIX_AIO
    char path[] = "/tmp/metaposix-aio-XXXXXX";
    char content[] = "asynchronous file write\n";
    char readback[sizeof content];
    struct aiocb request = { 0 };
    const struct aiocb *requests[] = { &request };
    int fd = -1;
    int result = -1;
    int saved_errno = 0;
    int aio_status;

    fd = mkstemp(path);
    if (fd == -1)
        goto error;
    if (unlink(path) == -1)
        goto error;

    request.aio_fildes = fd;
    request.aio_buf = content;
    request.aio_nbytes = sizeof content;
    request.aio_offset = 0;
    if (aio_write(&request) == -1)
        goto error;

    do {
        aio_status = aio_error(&request);
        if (aio_status == EINPROGRESS &&
            aio_suspend(requests, 1, NULL) == -1 && errno != EINTR)
            goto error;
    } while (aio_status == EINPROGRESS);
    if (aio_status != 0) {
        errno = aio_status;
        goto error;
    }
    if (aio_return(&request) != (ssize_t)sizeof content) {
        if (errno == 0)
            errno = EIO;
        goto error;
    }
    if (pread(fd, readback, sizeof readback, 0) != (ssize_t)sizeof readback ||
        memcmp(content, readback, sizeof content) != 0) {
        if (errno == 0)
            errno = EIO;
        goto error;
    }

    result = 0;
    puts("PASS POSIX asynchronous file I/O (aio_write/aio_suspend)");
    goto cleanup;

error:
    saved_errno = errno;
cleanup:
    if (fd != -1)
        close(fd);
    if (result != 0)
        fprintf(stderr, "FAIL asynchronous file I/O: %s\n",
                strerror(saved_errno != 0 ? saved_errno : EIO));
    return result;
#else
    puts("SKIP POSIX asynchronous file I/O: <aio.h> unavailable");
    return 0;
#endif
}
