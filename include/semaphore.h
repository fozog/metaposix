/*
 * metaposix interposed <semaphore.h>
 *
 * Pulls in the platform header, then — on platforms where unnamed
 * semaphores are broken (macOS: sem_init() returns ENOSYS and
 * sem_timedwait() does not exist) — transparently reroutes the
 * sem_* API to a mutex+condvar implementation (src/mp_sem.c).
 *
 * Limitation of the transparent mapping: named semaphores (sem_open)
 * are NOT rerouted; do not mix sem_open handles with sem_init/sem_wait
 * in shimmed code on macOS.
 */
#ifndef METAPOSIX_SEMAPHORE_H
#define METAPOSIX_SEMAPHORE_H

#include_next <semaphore.h>

#include <metaposix/mp_features.h>

#if !MP_HAS_WORKING_SEM_INIT

#include <pthread.h>
#include <time.h>

typedef struct mp_sem {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    unsigned        count;
} mp_sem_t;

int mp_sem_init(mp_sem_t *sem, int pshared, unsigned value);
int mp_sem_destroy(mp_sem_t *sem);
int mp_sem_wait(mp_sem_t *sem);
int mp_sem_trywait(mp_sem_t *sem);
int mp_sem_timedwait(mp_sem_t *sem, const struct timespec *abstime);
int mp_sem_post(mp_sem_t *sem);
int mp_sem_getvalue(mp_sem_t *sem, int *value);

/* Transparent source-level rerouting. */
#define sem_t         mp_sem_t
#define sem_init      mp_sem_init
#define sem_destroy   mp_sem_destroy
#define sem_wait      mp_sem_wait
#define sem_trywait   mp_sem_trywait
#define sem_timedwait mp_sem_timedwait
#define sem_post      mp_sem_post
#define sem_getvalue  mp_sem_getvalue

#endif /* !MP_HAS_WORKING_SEM_INIT */

#endif /* METAPOSIX_SEMAPHORE_H */
