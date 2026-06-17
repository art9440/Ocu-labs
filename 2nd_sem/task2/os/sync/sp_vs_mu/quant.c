#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>

static inline uint64_t nsec_now() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

int main() {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(10, &set);
    sched_setaffinity(0, sizeof(set), &set);

    printf("Measuring timeslice...\n");

    uint64_t last = nsec_now();
    while (1) {
        uint64_t now = nsec_now();
        uint64_t delta = now - last;
        last = now;

        if (delta > 1000000)  // >1ms
            printf("Switch gap: %llu us\n", (unsigned long long)(delta / 1000));
    }
}
