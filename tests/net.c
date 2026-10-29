#define _POSIX_C_SOURCE 200809L
#if defined(__APPLE__)
#define _DARWIN_C_SOURCE
#endif

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "test.h"

static int
test_tcp_sockets(void)
{
    const char message[] = "metaposix loopback";
    char received[sizeof message];
    struct sockaddr_in address = { 0 };
    socklen_t address_length = sizeof address;
    int listener = -1;
    int client = -1;
    int server = -1;
    int result = -1;
    int saved_errno = 0;
    size_t sent = 0;
    size_t total = 0;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1)
        goto error;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;
    if (bind(listener, (struct sockaddr *)&address, sizeof address) == -1 ||
        listen(listener, 1) == -1 ||
        getsockname(listener, (struct sockaddr *)&address, &address_length) == -1)
        goto error;

    client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == -1 ||
        connect(client, (struct sockaddr *)&address, sizeof address) == -1)
        goto error;
    server = accept(listener, NULL, NULL);
    if (server == -1)
        goto error;

    while (sent < sizeof message) {
        ssize_t n = send(client, message + sent, sizeof message - sent, 0);
        if (n == -1 && errno == EINTR)
            continue;
        if (n <= 0) {
            if (n == 0)
                errno = EIO;
            goto error;
        }
        sent += (size_t)n;
    }
    while (total < sizeof received) {
        ssize_t n = recv(server, received + total, sizeof received - total, 0);
        if (n == -1 && errno == EINTR)
            continue;
        if (n <= 0) {
            if (n == 0)
                errno = EIO;
            goto error;
        }
        total += (size_t)n;
    }
    if (memcmp(message, received, sizeof message) != 0) {
        errno = EIO;
        goto error;
    }

    result = 0;
    puts("PASS IPv4 TCP loopback sockets");
    goto cleanup;

error:
    saved_errno = errno;
cleanup:
    if (server != -1)
        close(server);
    if (client != -1)
        close(client);
    if (listener != -1)
        close(listener);
    if (result != 0)
        fprintf(stderr, "FAIL TCP socket test: %s\n",
                strerror(saved_errno != 0 ? saved_errno : EIO));
    return result;
}

static int
test_nonblocking_socket_io(void)
{
    const char message[] = "event-driven socket data";
    char received[sizeof message];
    struct pollfd descriptor;
    int sockets[2] = { -1, -1 };
    int flags;
    int result = -1;
    int saved_errno = 0;
    ssize_t n;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1)
        goto error;
    flags = fcntl(sockets[1], F_GETFL);
    if (flags == -1 || fcntl(sockets[1], F_SETFL, flags | O_NONBLOCK) == -1)
        goto error;

    descriptor.fd = sockets[1];
    descriptor.events = POLLIN;
    descriptor.revents = 0;
    n = poll(&descriptor, 1, 0);
    if (n == -1)
        goto error;
    if (n != 0 || descriptor.revents != 0) {
        errno = EIO;
        goto error;
    }

    {
        size_t sent = 0;
        while (sent < sizeof message) {
            n = write(sockets[0], message + sent, sizeof message - sent);
            if (n == -1 && errno == EINTR)
                continue;
            if (n <= 0) {
                if (n == 0)
                    errno = EIO;
                goto error;
            }
            sent += (size_t)n;
        }
    }
    do {
        n = poll(&descriptor, 1, 1000);
    } while (n == -1 && errno == EINTR);
    if (n != 1 || !(descriptor.revents & POLLIN)) {
        if (n == 0)
            errno = ETIMEDOUT;
        else if (n > 0)
            errno = EIO;
        goto error;
    }

    do {
        n = read(sockets[1], received, sizeof received);
    } while (n == -1 && errno == EINTR);
    if (n != (ssize_t)sizeof received ||
        memcmp(message, received, sizeof message) != 0) {
        if (n >= 0)
            errno = EIO;
        goto error;
    }

    result = 0;
    puts("PASS nonblocking socket I/O with poll readiness");
    goto cleanup;

error:
    saved_errno = errno;
cleanup:
    if (sockets[0] != -1)
        close(sockets[0]);
    if (sockets[1] != -1)
        close(sockets[1]);
    if (result != 0)
        fprintf(stderr, "FAIL asynchronous socket I/O: %s\n",
                strerror(saved_errno != 0 ? saved_errno : EIO));
    return result;
}

int
mp_test_net(void)
{
    if (test_tcp_sockets() != 0)
        return -1;
    return test_nonblocking_socket_io();
}
