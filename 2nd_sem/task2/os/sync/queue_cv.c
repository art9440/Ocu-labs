#define _GNU_SOURCE
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"

typedef struct {
    queue_t q;
    pthread_mutex_t m;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} qwrap_t;

static inline qwrap_t *W(queue_t *q) { return (qwrap_t*)q; }

static void *qmonitor(void *arg) {
    queue_t *q = (queue_t*)arg;
    printf("qmonitor: [%d %d %ld]\n", getpid(), getppid(), (long)gettid());
    while (1) {
        queue_print_stats(q);
        sleep(1);
    }
    return NULL;
}

queue_t* queue_init(int max_count) {
    qwrap_t *w = calloc(1, sizeof(*w));
    if (!w) { perror("calloc"); abort(); }

    queue_t *q = &w->q;
    q->max_count = max_count;

    pthread_mutex_init(&w->m, NULL);
    pthread_cond_init(&w->not_full, NULL);
    pthread_cond_init(&w->not_empty, NULL);

    pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);
    return q;
}

void queue_destroy(queue_t *q) {
    qwrap_t *w = W(q);
    pthread_cancel(q->qmonitor_tid);
    pthread_join(q->qmonitor_tid, NULL);

    pthread_mutex_lock(&w->m);
    for (qnode_t *n = q->first; n;) {
        qnode_t *next = n->next;
        free(n);
        n = next;
    }
    pthread_mutex_unlock(&w->m);

    pthread_mutex_destroy(&w->m);
    pthread_cond_destroy(&w->not_full);
    pthread_cond_destroy(&w->not_empty);
    free(w);
}

int queue_add(queue_t *q, int val) {
    qwrap_t *w = W(q);
    pthread_mutex_lock(&w->m);

    q->add_attempts++;
    while (q->count == q->max_count)
        pthread_cond_wait(&w->not_full, &w->m);

    qnode_t *n = malloc(sizeof(*n));
    if (!n) { perror("malloc"); abort(); }
    n->val = val;
    n->next = NULL;

    if (!q->first) q->first = q->last = n;
    else q->last = q->last->next = n;

    q->count++;
    q->add_count++;

    pthread_cond_signal(&w->not_empty);  // будим читателя
    pthread_mutex_unlock(&w->m);
    return 1;
}

int queue_get(queue_t *q, int *val) {
    qwrap_t *w = W(q);
    pthread_mutex_lock(&w->m);

    q->get_attempts++;
    while (q->count == 0)
        pthread_cond_wait(&w->not_empty, &w->m);

    qnode_t *tmp = q->first;
    *val = tmp->val;
    q->first = tmp->next;
    if (!q->first) q->last = NULL;
    free(tmp);

    q->count--;
    q->get_count++;

    pthread_cond_signal(&w->not_full);   // будим писателя
    pthread_mutex_unlock(&w->m);
    return 1;
}

void queue_print_stats(queue_t *q) {
    qwrap_t *w = W(q);
    pthread_mutex_lock(&w->m);
    printf("queue stats: current size %d; attempts: (%ld %ld %ld); counts (%ld %ld %ld)\n",
        q->count,
        q->add_attempts, q->get_attempts, q->add_attempts - q->get_attempts,
        q->add_count, q->get_count, q->add_count - q->get_count);
    pthread_mutex_unlock(&w->m);
}
