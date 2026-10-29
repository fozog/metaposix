#define _POSIX_C_SOURCE 200809L
#if defined(__APPLE__)
#define _DARWIN_C_SOURCE
#endif

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "test.h"

int
mp_test_file_fs(void)
{
    char directory[] = "/tmp/metaposix-dir-XXXXXX";
    char filename[sizeof directory + sizeof "/sample.txt"];
    const char contents[] = "filesystem test\n";
    char readback[sizeof contents];
    struct stat status;
    DIR *dir = NULL;
    struct dirent *entry;
    int fd = -1;
    int directory_ready = 0;
    int file_ready = 0;
    int found = 0;
    int result = -1;
    int saved_errno = 0;
    size_t written = 0;
    size_t received = 0;

    if (mkdtemp(directory) == NULL)
        goto error;
    directory_ready = 1;

    if (snprintf(filename, sizeof filename, "%s/sample.txt", directory) < 0) {
        errno = EIO;
        goto error;
    }
    fd = open(filename, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
        goto error;
    file_ready = 1;

    while (written < sizeof contents) {
        ssize_t n = write(fd, contents + written, sizeof contents - written);
        if (n == -1 && errno == EINTR)
            continue;
        if (n <= 0) {
            if (n == 0)
                errno = EIO;
            goto error;
        }
        written += (size_t)n;
    }

    if (fstat(fd, &status) == -1)
        goto error;
    if (!S_ISREG(status.st_mode) || status.st_size != (off_t)sizeof contents) {
        errno = EIO;
        goto error;
    }
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1)
        goto error;

    while (received < sizeof readback) {
        ssize_t n = read(fd, readback + received, sizeof readback - received);
        if (n == -1 && errno == EINTR)
            continue;
        if (n <= 0) {
            if (n == 0)
                errno = EIO;
            goto error;
        }
        received += (size_t)n;
    }
    if (memcmp(contents, readback, sizeof contents) != 0) {
        errno = EIO;
        goto error;
    }

    dir = opendir(directory);
    if (dir == NULL)
        goto error;
    errno = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, "sample.txt") == 0)
            found = 1;
    }
    if (errno != 0)
        goto error;
    if (!found) {
        errno = ENOENT;
        goto error;
    }

    result = 0;
    puts("PASS file creation, read/write, metadata, and directory traversal");
    goto cleanup;

error:
    saved_errno = errno;
cleanup:
    if (dir != NULL)
        closedir(dir);
    if (fd != -1)
        close(fd);
    if (file_ready && unlink(filename) == -1 && result == 0) {
        saved_errno = errno;
        result = -1;
    }
    if (directory_ready && rmdir(directory) == -1 && result == 0) {
        saved_errno = errno;
        result = -1;
    }
    if (result != 0)
        fprintf(stderr, "FAIL filesystem test: %s\n",
                strerror(saved_errno != 0 ? saved_errno : EIO));
    return result;
}
