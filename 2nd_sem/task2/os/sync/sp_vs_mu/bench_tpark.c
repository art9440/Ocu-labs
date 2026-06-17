// bench_tpark.c
#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>

static int cmp_u64(const void *a, const void *b) {
    uint64_t x = *(const uint64_t*)a;
    uint64_t y = *(const uint64_t*)b;
    /* возвращаем -1/0/1 без переполнений */
    return (x > y) - (x < y);
}

static inline uint64_t nsec_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  c = PTHREAD_COND_INITIALIZER;

static int ITERS = 200000;

static void set_cpu(int n) {
    cpu_set_t s; CPU_ZERO(&s); CPU_SET(n, &s);
    pthread_setaffinity_np(pthread_self(), sizeof(s), &s);
}

static uint64_t *samples;

static unsigned seq = 0;   // глобальный "номер события"

void* waiter(void *arg) {
    set_cpu(0);
    unsigned seen = 0;     // сколько событий уже обработали

    for (unsigned i = 0; i < ITERS; i++) {
        pthread_mutex_lock(&m);
        uint64_t t0 = nsec_now();

        while (seq == seen)            // ждём НОВОЕ событие
            pthread_cond_wait(&c, &m);

        seen = seq;                    // зафиксировали, что увидели новое
        uint64_t t1 = nsec_now();
        samples[i] = t1 - t0;

        pthread_mutex_unlock(&m);
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc > 1) ITERS = atoi(argv[1]);
    samples = calloc(ITERS, sizeof(uint64_t));
    if (!samples) { perror("calloc"); return 1; }

    pthread_t th;
    pthread_create(&th, NULL, waiter, NULL);
    set_cpu(1);

    usleep(1000); // чтоб waiter успел начать, но уже не критично

    for (unsigned i = 0; i < ITERS; i++) {
        pthread_mutex_lock(&m);
        seq++;                      // каждое событие — новый номер
        pthread_cond_signal(&c);    // будим
        pthread_mutex_unlock(&m);
    }

    pthread_join(th, NULL);

    qsort(samples, ITERS, sizeof(uint64_t), cmp_u64);
    uint64_t p50  = samples[(size_t)(ITERS * 0.50)];
    uint64_t p95  = samples[(size_t)(ITERS * 0.95)];
    uint64_t p99  = samples[(size_t)(ITERS * 0.99)];
    uint64_t p999 = samples[(size_t)(ITERS * 0.999)];
    printf("Tpark (cond_wait wake) [ns]: p50=%llu  p95=%llu  p99=%llu  p99.9=%llu\n",
           (unsigned long long)p50,
           (unsigned long long)p95,
           (unsigned long long)p99,
           (unsigned long long)p999);
    return 0;
}
