#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void *thread_fn(void *arg) {
    // Трогаем 1 МБ стека, чтобы реально занялась память
    volatile char buf[1<<20];
    memset((void*)buf, 0xA5, sizeof(buf));
    return NULL;
}

static long read_rss_kb(void) {
    FILE *f = fopen("/proc/self/statm", "r");
    if (!f) return -1;

    long size_pages = 0, rss_pages = 0;
    if (fscanf(f, "%ld %ld", &size_pages, &rss_pages) != 2) {
        fclose(f);
        return -1;
    }
    fclose(f);

    long page_kb = sysconf(_SC_PAGESIZE) / 1024;
    return rss_pages * page_kb;
}


int main(void) {
    pthread_t t;
    long rss_before, rss_after;

    printf("До создания потока:\n");
    rss_before = read_rss_kb();
    printf("VmRSS = %ld kB\n", rss_before);

    pthread_create(&t, NULL, thread_fn, NULL);
    usleep(100000); // дождаться завершения потока

    rss_after = read_rss_kb();
    printf("После завершения потока (без join):\n");
    printf("VmRSS = %ld kB (∆ %+ld kB)\n",
           rss_after, rss_after - rss_before);

    getchar(); // ждем — можно посмотреть через smaps/pmap

    pthread_join(t, NULL);

    long rss_final = read_rss_kb();
    printf("После join:\n");
    printf("VmRSS = %ld kB (∆ %+ld kB)\n",
           rss_final, rss_final - rss_before);
}
