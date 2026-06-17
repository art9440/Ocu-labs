#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <stdlib.h>

static void *thread_loop(void *arg) {
    pid_t tid = gettid();
    printf("loop thread tid=%d\n", tid);
    fflush(stdout);
    
    return NULL;
}

static int read_meminfo_kb(int *mem_avail_kb,
                           int *swap_free_kb)
{
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return -1;
    char key[64];
    int val;
    char unit[16];
    int ma = 0, sf = 0;

    while (fscanf(f, "%63s %d %15s", key, &val, unit) == 3) {
        if (strcmp(key, "MemAvailable:") == 0) ma = val;
        else if (strcmp(key, "SwapFree:") == 0) sf = val;
        if (ma && sf) break;
    }

    fclose(f);
    *mem_avail_kb = ma;
    *swap_free_kb = sf;
    return 0;
}

int main(void) {
    int err;
    int base_mem_kb = 0, base_swap_kb = 0;

    read_meminfo_kb(&base_mem_kb, &base_swap_kb);
    int base_total_kb = base_mem_kb + base_swap_kb;

    int c = 1;
    while (1) {

        pthread_t t;
        err = pthread_create(&t, NULL, thread_loop, NULL);
        if (err) {
            fprintf(stderr, "loop: pthread_create() failed: %s\n", strerror(err));
            return 1;
        }

        int cur_mem_kb = 0, cur_swap_kb = 0;
        read_meminfo_kb(&cur_mem_kb, &cur_swap_kb);
        int cur_total_kb = cur_mem_kb + cur_swap_kb;


        int used_kb = 0;
        double used_pct = 0.0;


        used_kb = base_total_kb - cur_total_kb;
        if (used_kb < 0) used_kb = 0;
        used_pct = 100.0 * (double)used_kb / (double)base_total_kb;
        if (used_pct < 0.0) used_pct = 0.0;
        if (used_pct > 100.0) used_pct = 100.0;
        

        printf("%d. base_total(RAM+swap)=%d KiB  used=%.2f%%(%d)\n", c, base_total_kb, used_pct, used_kb);
        fflush(stdout);

        c++;
    }

    return 0;
}
