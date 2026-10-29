#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>

#include "test.h"

struct thread_context {
    sem_t gate;
    pthread_mutex_t mutex;
    int value;
    int worker_error;
};

static void *
thread_worker(void *arg)
{
    struct thread_context *context = arg;
    int rc;

    while (sem_wait(&context->gate) == -1) {
        if (errno == EINTR)
            continue;
        context->worker_error = errno;
        return NULL;
    }

    rc = pthread_mutex_lock(&context->mutex);
    if (rc != 0) {
        context->worker_error = rc;
        return NULL;
    }
    context->value++;
    rc = pthread_mutex_unlock(&context->mutex);
    if (rc != 0)
        context->worker_error = rc;
    return NULL;
}

int
mp_test_thread_sync(void)
{
    struct thread_context context = { 0 };
    pthread_t thread;
    int mutex_ready = 0;
    int sem_ready = 0;
    int thread_ready = 0;
    int result = -1;
    int rc;

    rc = pthread_mutex_init(&context.mutex, NULL);
    if (rc != 0) {
        fprintf(stderr, "FAIL pthread mutex init: %s\n", strerror(rc));
        goto cleanup;
    }
    mutex_ready = 1;

    if (sem_init(&context.gate, 0, 0) == -1) {
        fprintf(stderr, "FAIL semaphore init: %s\n", strerror(errno));
        goto cleanup;
    }
    sem_ready = 1;

    rc = pthread_create(&thread, NULL, thread_worker, &context);
    if (rc != 0) {
        fprintf(stderr, "FAIL pthread create: %s\n", strerror(rc));
        goto cleanup;
    }
    thread_ready = 1;

    if (sem_post(&context.gate) == -1) {
        fprintf(stderr, "FAIL semaphore post: %s\n", strerror(errno));
        pthread_cancel(thread);
        goto cleanup;
    }

    rc = pthread_join(thread, NULL);
    if (rc != 0) {
        fprintf(stderr, "FAIL pthread join: %s\n", strerror(rc));
        thread_ready = 0;
        goto cleanup;
    }
    thread_ready = 0;

    if (context.worker_error != 0) {
        fprintf(stderr, "FAIL worker synchronization: %s\n",
                strerror(context.worker_error));
        goto cleanup;
    }
    if (context.value != 1) {
        fprintf(stderr, "FAIL mutex-protected counter: got %d\n",
                context.value);
        goto cleanup;
    }

    puts("PASS pthread creation, mutex, and semaphore synchronization");
    result = 0;

cleanup:
    if (thread_ready) {
        if (sem_post(&context.gate) == -1)
            pthread_cancel(thread);
        pthread_join(thread, NULL);
    }
    if (sem_ready)
        sem_destroy(&context.gate);
    if (mutex_ready)
        pthread_mutex_destroy(&context.mutex);
    return result;
}
