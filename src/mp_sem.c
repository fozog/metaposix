/*
 * metaposix shim: unnamed POSIX semaphores for platforms where
 * sem_init() is broken (macOS returns ENOSYS).
 *
 * Pure pthread mutex+condvar counting semaphore.  Matches POSIX
 * semantics for intra-process use; pshared != 0 is reported as
 * unsupported (ENOSYS), which mirrors what the platform itself
 * cannot do.
 */
#include <semaphore.h>

#if !MP_HAS_WORKING_SEM_INIT

#include <errno.h>
#include <limits.h>

int mp_sem_init(mp_sem_t *sem, int pshared, unsigned value)
{
    if (pshared != 0) {
        errno = ENOSYS;
        return -1;
    }
    if (value > (unsigned)INT_MAX) {
        errno = EINVAL;
        return -1;
    }
    int rc = pthread_mutex_init(&sem->mutex, NULL);
    if (rc != 0) { errno = rc; return -1; }
    rc = pthread_cond_init(&sem->cond, NULL);
    if (rc != 0) {
        pthread_mutex_destroy(&sem->mutex);
        errno = rc;
        return -1;
    }
    sem->count = value;
    return 0;
}

int mp_sem_destroy(mp_sem_t *sem)
{
    pthread_mutex_destroy(&sem->mutex);
    pthread_cond_destroy(&sem->cond);
    return 0;
}

int mp_sem_wait(mp_sem_t *sem)
{
    pthread_mutex_lock(&sem->mutex);
    while (sem->count == 0)
        pthread_cond_wait(&sem->cond, &sem->mutex);
    sem->count--;
    pthread_mutex_unlock(&sem->mutex);
    return 0;
}

int mp_sem_trywait(mp_sem_t *sem)
{
    int result = 0;
    pthread_mutex_lock(&sem->mutex);
    if (sem->count > 0) {
        sem->count--;
    } else {
        errno = EAGAIN;
        result = -1;
    }
    pthread_mutex_unlock(&sem->mutex);
    return result;
}

int mp_sem_timedwait(mp_sem_t *sem, const struct timespec *abstime)
{
    if (abstime == NULL ||
        abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000L) {
        errno = EINVAL;
        return -1;
    }
    int result = 0;
    pthread_mutex_lock(&sem->mutex);
    while (sem->count == 0) {
        int rc = pthread_cond_timedwait(&sem->cond, &sem->mutex, abstime);
        if (rc == ETIMEDOUT) {
            errno = ETIMEDOUT;
            result = -1;
            break;
        }
    }
    if (result == 0)
        sem->count--;
    pthread_mutex_unlock(&sem->mutex);
    return result;
}

int mp_sem_post(mp_sem_t *sem)
{
    pthread_mutex_lock(&sem->mutex);
    if (sem->count == (unsigned)INT_MAX) {
        pthread_mutex_unlock(&sem->mutex);
        errno = EOVERFLOW;
        return -1;
    }
    sem->count++;
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);
    return 0;
}

int mp_sem_getvalue(mp_sem_t *sem, int *value)
{
    pthread_mutex_lock(&sem->mutex);
    *value = (int)sem->count;
    pthread_mutex_unlock(&sem->mutex);
    return 0;
}

#endif /* !MP_HAS_WORKING_SEM_INIT */
