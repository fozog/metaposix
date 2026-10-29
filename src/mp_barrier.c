/*
 * metaposix shim: pthread barriers (_POSIX_BARRIERS) for platforms
 * that lack them (macOS).
 *
 * Generation-counted mutex+condvar barrier; reusable across cycles,
 * and exactly one waiter per cycle receives
 * PTHREAD_BARRIER_SERIAL_THREAD, as POSIX requires.
 */
#include "metaposix.h"

#include <pthread.h>

#if !MP_HAS_PTHREAD_BARRIER

#include <errno.h>

int mp_pthread_barrier_init(mp_pthread_barrier_t *barrier,
                            const mp_pthread_barrierattr_t *attr,
                            unsigned count)
{
    (void)attr;
    if (count == 0)
        return EINVAL;
    int rc = pthread_mutex_init(&barrier->mutex, NULL);
    if (rc != 0)
        return rc;
    rc = pthread_cond_init(&barrier->cond, NULL);
    if (rc != 0) {
        pthread_mutex_destroy(&barrier->mutex);
        return rc;
    }
    barrier->threshold = count;
    barrier->waiting = 0;
    barrier->generation = 0;
    return 0;
}

int mp_pthread_barrier_destroy(mp_pthread_barrier_t *barrier)
{
    pthread_mutex_destroy(&barrier->mutex);
    pthread_cond_destroy(&barrier->cond);
    return 0;
}

int mp_pthread_barrier_wait(mp_pthread_barrier_t *barrier)
{
    pthread_mutex_lock(&barrier->mutex);
    unsigned gen = barrier->generation;
    if (++barrier->waiting == barrier->threshold) {
        barrier->generation++;
        barrier->waiting = 0;
        pthread_cond_broadcast(&barrier->cond);
        pthread_mutex_unlock(&barrier->mutex);
        return PTHREAD_BARRIER_SERIAL_THREAD;
    }
    while (gen == barrier->generation)
        pthread_cond_wait(&barrier->cond, &barrier->mutex);
    pthread_mutex_unlock(&barrier->mutex);
    return 0;
}

int mp_pthread_barrierattr_init(mp_pthread_barrierattr_t *attr)
{
    attr->pshared = 0;
    return 0;
}

int mp_pthread_barrierattr_destroy(mp_pthread_barrierattr_t *attr)
{
    (void)attr;
    return 0;
}

#endif /* !MP_HAS_PTHREAD_BARRIER */
