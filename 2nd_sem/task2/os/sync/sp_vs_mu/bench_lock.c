#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>

static inline uint64_t nsec_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

static void set_cpu(int n) {
    cpu_set_t s;
    CPU_ZERO(&s);
    CPU_SET(n, &s);
    pthread_setaffinity_np(pthread_self(), sizeof(s), &s);
}

/*** ПАРАМЕТР: сколько "работы" внутри критической секции ***/
static int WORK_ITERS = 0;

/*** ЛОК: либо мьютекс, либо спинлок — выбираем макросом ***/
#ifdef USE_SPIN
static pthread_spinlock_t lk;
#else
static pthread_mutex_t lk = PTHREAD_MUTEX_INITIALIZER;
#endif

static volatile int stop_flag = 0;
static unsigned long long ops[2] = {0, 0};

static inline void lock_lock(void) {
#ifdef USE_SPIN
    pthread_spin_lock(&lk);
#else
    pthread_mutex_lock(&lk);
#endif
}

static inline void lock_unlock(void) {
#ifdef USE_SPIN
    pthread_spin_unlock(&lk);
#else
    pthread_mutex_unlock(&lk);
#endif
}

void *worker(void *arg) {
    long id = (long)arg;
    set_cpu((int)id);   // 0 и 1 CPU

    unsigned long long local = 0;

    while (!stop_flag) {
        lock_lock();

        // Эмулируем работу в секции
        for (int i = 0; i < WORK_ITERS; i++) {
            __asm__ __volatile__("" ::: "memory");
        }

        lock_unlock();
        local++;
    }

    ops[id] = local;
    return NULL;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        WORK_ITERS = atoi(argv[1]);
    }

#ifdef USE_SPIN
    if (pthread_spin_init(&lk, PTHREAD_PROCESS_PRIVATE) != 0) {
        perror("pthread_spin_init");
        return 1;
    }
#else
    // mutex уже инициализирован статически
#endif

    pthread_t t0, t1;
    pthread_create(&t0, NULL, worker, (void*)0L);
    pthread_create(&t1, NULL, worker, (void*)1L);

    // даём поработать 5 секунд
    uint64_t start = nsec_now();
    sleep(5);
    stop_flag = 1;

    pthread_join(t0, NULL);
    pthread_join(t1, NULL);

    uint64_t end = nsec_now();
    double sec = (end - start) / 1e9;

    unsigned long long total_ops = ops[0] + ops[1];
#ifdef USE_SPIN
    printf("[SPIN]  WORK_ITERS=%d  ops=%llu  ops/sec=%.0f\n",
           WORK_ITERS, total_ops, total_ops / sec);
#else
    printf("[MUTEX] WORK_ITERS=%d  ops=%llu  ops/sec=%.0f\n",
           WORK_ITERS, total_ops, total_ops / sec);
#endif

#ifdef USE_SPIN
    pthread_spin_destroy(&lk);
#endif

    return 0;
}
