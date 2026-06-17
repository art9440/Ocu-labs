#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <sched.h>
#include <unistd.h>

#define ITERATIONS 1000000

volatile int turn = 0;
volatile int thread_ready = 0;

void set_affinity(int cpu) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

void* worker(void* arg) {
    set_affinity(0);
    for (int i = 0; i < ITERATIONS; i++) {
            sched_yield();
    }    
    return NULL;
}

int main() {
    pthread_t t;
    struct timeval start, end;
    set_affinity(0);
    pthread_create(&t, NULL, worker, NULL);
    gettimeofday(&start, NULL);
    for (int i = 0; i < ITERATIONS; i++) {
            sched_yield();
    }
    gettimeofday(&end, NULL);
    pthread_join(t, NULL);
    long long total_us = (end.tv_sec - start.tv_sec) * 1000000LL + 
                        (end.tv_usec - start.tv_usec);
    double avg_ns_per_switch = (total_us * 1000.0) / ITERATIONS;
    printf("Total time: %lld us\n", total_us);
    printf("Average: %.2f ns per context switch\n", avg_ns_per_switch);
    
    
    return 0;
}