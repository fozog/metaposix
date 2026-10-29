/*
 * metaposix shim: ppoll() (POSIX.1-2024) for platforms without it
 * (macOS, NetBSD).
 *
 * Emulated with pthread_sigmask() + poll().  The mask swap and the
 * wait are NOT atomic as they are in a native ppoll; a signal can
 * slip in between.  MP_HAS_PPOLL == 0 lets callers know they are on
 * the emulated path.
 */
#include <poll.h>

#if !MP_HAS_PPOLL

#include <errno.h>
#include <limits.h>

int mp_ppoll(struct pollfd *fds, nfds_t nfds,
             const struct timespec *timeout, const sigset_t *sigmask)
{
    int timeout_ms;

    if (timeout == NULL) {
        timeout_ms = -1;
    } else if (timeout->tv_sec < 0 ||
               timeout->tv_nsec < 0 || timeout->tv_nsec >= 1000000000L) {
        errno = EINVAL;
        return -1;
    } else if (timeout->tv_sec > (INT_MAX / 1000) - 1) {
        timeout_ms = INT_MAX;
    } else {
        /* Round up so we never return earlier than requested. */
        timeout_ms = (int)(timeout->tv_sec * 1000 +
                           (timeout->tv_nsec + 999999L) / 1000000L);
    }

    sigset_t origmask;
    if (sigmask != NULL)
        pthread_sigmask(SIG_SETMASK, sigmask, &origmask);

    int ready = poll(fds, nfds, timeout_ms);
    int saved_errno = errno;

    if (sigmask != NULL)
        pthread_sigmask(SIG_SETMASK, &origmask, NULL);

    errno = saved_errno;
    return ready;
}

#endif /* !MP_HAS_PPOLL */
