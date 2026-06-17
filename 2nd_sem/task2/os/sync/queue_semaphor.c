#define _GNU_SOURCE
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include "queue.h"

typedef struct {
    queue_t q;
    sem_t mutex;
    sem_t items;
    sem_t spaces;
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

    sem_init(&w->mutex, 0, 1);
    sem_init(&w->items, 0, 0);
    sem_init(&w->spaces, 0, max_count);

    pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);
    return q;
}

void queue_destroy(queue_t *q) {
    qwrap_t *w = W(q);
    pthread_cancel(q->qmonitor_tid);
    pthread_join(q->qmonitor_tid, NULL);

    sem_wait(&w->mutex);
    for (qnode_t *n = q->first; n;) {
        qnode_t *next = n->next;
        free(n);
        n = next;
    }
    sem_post(&w->mutex);

    sem_destroy(&w->mutex);
    sem_destroy(&w->items);
    sem_destroy(&w->spaces);
    free(w);
}

int queue_add(queue_t *q, int val) {
    qwrap_t *w = W(q);
    sem_wait(&w->spaces);   // жди свободное место
    sem_wait(&w->mutex);    // захват очереди

    q->add_attempts++;

    qnode_t *n = malloc(sizeof(*n));
    if (!n) { perror("malloc"); abort(); }
    n->val = val;
    n->next = NULL;

    if (!q->first) q->first = q->last = n;
    else q->last = q->last->next = n;

    q->count++;
    q->add_count++;

    sem_post(&w->mutex);
    sem_post(&w->items);    // сигнал, что есть элемент
    return 1;
}

int queue_get(queue_t *q, int *val) {
    qwrap_t *w = W(q);
    sem_wait(&w->items);    // жди пока есть элементы
    sem_wait(&w->mutex);

    q->get_attempts++;

    qnode_t *tmp = q->first;
    *val = tmp->val;
    q->first = tmp->next;
    if (!q->first) q->last = NULL;
    free(tmp);

    q->count--;
    q->get_count++;

    sem_post(&w->mutex);
    sem_post(&w->spaces);   // сигнал: появилось место
    return 1;
}

void queue_print_stats(queue_t *q) {
    qwrap_t *w = W(q);
    sem_wait(&w->mutex);
    printf("queue stats: current size %d; attempts: (%ld %ld %ld); counts (%ld %ld %ld)\n",
        q->count,
        q->add_attempts, q->get_attempts, q->add_attempts - q->get_attempts,
        q->add_count, q->get_count, q->add_count - q->get_count);
    sem_post(&w->mutex);
}
