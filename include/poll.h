/*
 * metaposix interposed <poll.h>
 *
 * Supplies ppoll() — standardized by POSIX.1-2024 — on platforms that
 * lack it (macOS, NetBSD).  Implementation in src/mp_ppoll.c.
 *
 * Fidelity note: the emulation (pthread_sigmask + poll) cannot make the
 * sigmask swap and the wait atomic the way a native ppoll can.  That is
 * a documented, irreducible platform limitation — exactly the category
 * of divergence metaposix reports via MP_HAS_PPOLL instead of hiding.
 */
#ifndef METAPOSIX_POLL_H
#define METAPOSIX_POLL_H

#include_next <poll.h>

#include <metaposix/mp_features.h>

#if !MP_HAS_PPOLL

#include <signal.h>
#include <time.h>

int mp_ppoll(struct pollfd *fds, nfds_t nfds,
             const struct timespec *timeout, const sigset_t *sigmask);

#define ppoll mp_ppoll

#endif /* !MP_HAS_PPOLL */

#endif /* METAPOSIX_POLL_H */
