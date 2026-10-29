/*
 * metaposix interposed <pthread.h>
 *
 * Pulls in the platform header, then supplies pthread barriers
 * (_POSIX_BARRIERS option) on platforms that lack them (macOS).
 * Implementation in src/mp_barrier.c.
 */
#ifndef METAPOSIX_PTHREAD_H
#define METAPOSIX_PTHREAD_H

#include_next <pthread.h>

#include <metaposix/mp_features.h>

#if !MP_HAS_PTHREAD_BARRIER && !defined(PTHREAD_BARRIER_SERIAL_THREAD)

typedef struct mp_pthread_barrier {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    unsigned        threshold;   /* required waiters                  */
    unsigned        waiting;     /* waiters in the current generation */
    unsigned        generation;  /* cycle counter                     */
} mp_pthread_barrier_t;

typedef struct mp_pthread_barrierattr {
    int pshared;
} mp_pthread_barrierattr_t;

#define PTHREAD_BARRIER_SERIAL_THREAD (-1)

int mp_pthread_barrier_init(mp_pthread_barrier_t *barrier,
                            const mp_pthread_barrierattr_t *attr,
                            unsigned count);
int mp_pthread_barrier_destroy(mp_pthread_barrier_t *barrier);
int mp_pthread_barrier_wait(mp_pthread_barrier_t *barrier);

int mp_pthread_barrierattr_init(mp_pthread_barrierattr_t *attr);
int mp_pthread_barrierattr_destroy(mp_pthread_barrierattr_t *attr);

/* Transparent source-level rerouting. */
#define pthread_barrier_t            mp_pthread_barrier_t
#define pthread_barrierattr_t        mp_pthread_barrierattr_t
#define pthread_barrier_init         mp_pthread_barrier_init
#define pthread_barrier_destroy      mp_pthread_barrier_destroy
#define pthread_barrier_wait         mp_pthread_barrier_wait
#define pthread_barrierattr_init     mp_pthread_barrierattr_init
#define pthread_barrierattr_destroy  mp_pthread_barrierattr_destroy

#endif /* !MP_HAS_PTHREAD_BARRIER */

#endif /* METAPOSIX_PTHREAD_H */
