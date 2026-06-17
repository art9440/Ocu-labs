
#define _GNU_SOURCE
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"

typedef struct {
    queue_t         q;   
    pthread_mutex_t m;
} qwrap_t;

static inline qwrap_t *W(queue_t *q) { return (qwrap_t*)q; }

static void *qmonitor(void *arg) {
    queue_t *q = (queue_t *)arg;
     printf("qmonitor: [%d %d %ld]\n", getpid(), getppid(), (long)gettid());
    while (1) {
        queue_print_stats(q);
        sleep(1);
    }
    return NULL;
}

queue_t* queue_init(int max_count) {
    qwrap_t *w = (qwrap_t*)calloc(1, sizeof(qwrap_t));
    if (!w) { perror("calloc queue"); abort(); }

    queue_t *q = &w->q;
    q->first = NULL;
    q->last  = NULL;
    q->max_count = max_count;
    q->count = 0;
    q->add_attempts = q->get_attempts = 0;
    q->add_count    = q->get_count    = 0;

    if (pthread_mutex_init(&w->m, NULL) != 0) { perror("pthread_mutex_init"); abort(); }

    int err = pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);
    if (err) { fprintf(stderr, "pthread_create: %s\n", strerror(err)); abort(); }
    return q;
}

void queue_destroy(queue_t *q) {
    if (!q) return;
    qwrap_t *w = W(q);

    pthread_cancel(q->qmonitor_tid);
    pthread_join(q->qmonitor_tid, NULL);

    pthread_mutex_lock(&w->m);
    qnode_t *n = q->first;
    while (n) { qnode_t *next = n->next; free(n); n = next; }
    pthread_mutex_unlock(&w->m);

    pthread_mutex_destroy(&w->m);
    free(w);
}

int queue_add(queue_t *q, int val) {
    qwrap_t *w = W(q);
    pthread_mutex_lock(&w->m);

    q->add_attempts++;
    assert(q->count <= q->max_count);
    if (q->count == q->max_count) {
        pthread_mutex_unlock(&w->m);
        return 0;
    }

    qnode_t *node = (qnode_t*)malloc(sizeof(*node));
    if (!node) { pthread_mutex_unlock(&w->m); perror("malloc node"); abort(); }
    node->val = val; node->next = NULL;

    if (!q->first) {
        q->first = q->last = node;
    } else {
        q->last->next = node;
        q->last = node;
    }
    q->count++;
    q->add_count++;

    pthread_mutex_unlock(&w->m);
    return 1;
}

int queue_get(queue_t *q, int *val) {
    qwrap_t *w = W(q);
    pthread_mutex_lock(&w->m);

    q->get_attempts++;
    if (q->count <= 0 || q->first == NULL) {
        pthread_mutex_unlock(&w->m);
        return 0;
    }

    qnode_t *tmp = q->first;
    *val = tmp->val;
    q->first = tmp->next;
    if (!q->first) q->last = NULL;
    free(tmp);

    if (q->count > 0) q->count--;
    if (q->get_count < q->add_count) q->get_count++;

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
