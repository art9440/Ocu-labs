// measure_tail.c
#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

/* Возвращает RSS (в KiB) по /proc/self/statm */
static long read_rss_kb_statm(void) {
    FILE *f = fopen("/proc/self/statm", "r");
    if (!f) return -1;
    long size_p = 0, rss_p = 0;
    if (fscanf(f, "%ld %ld", &size_p, &rss_p) != 2) { fclose(f); return -1; }
    fclose(f);
    long page_kb = sysconf(_SC_PAGESIZE) / 1024;
    return rss_p * page_kb;
}

/* Две пары пайпов для синхронизации:
   - worker -> main  (sync_from_worker): сообщает «жив» и «выхожу»
   - main   -> worker(sync_to_worker):  даём команду выйти */
static int sync_to_worker[2];
static int sync_from_worker[2];

static void *worker(void *arg) {
    (void)arg;

    /* 1) «Грязним» стек, чтобы было видно влияние на RSS. */
    volatile char stack_buf[4 << 20]; /* 4 MiB */
    memset((void*)stack_buf, 0xA5, sizeof(stack_buf));

    /* 2) Сообщаем main: «я жив, стек занят» */
    if (write(sync_from_worker[1], "S", 1) != 1) {
        /* ничего не делаем, это демо */
    }

    /* 3) Ждём разрешение на выход */
    char cmd;
    if (read(sync_to_worker[0], &cmd, 1) != 1) {
        /* тоже игнорируем ошибки в демо */
    }

    /* 4) Сообщаем main: «сейчас выхожу (return из start_routine)» */
    if (write(sync_from_worker[1], "E", 1) != 1) {
        /* ignore */
    }

    /* 5) Возврат — glibc освободит стек этого потока до pthread_join */
    return NULL;
}

int main(void) {
    pthread_t t;
    int r;
    char ch;

    if (pipe(sync_to_worker) != 0 || pipe(sync_from_worker) != 0) {
        perror("pipe");
        return 1;
    }

    printf("=== Measure RSS around thread lifecycle ===\n");

    long rss0 = read_rss_kb_statm();
    printf("RSS before create: %ld KiB\n", rss0);

    r = pthread_create(&t, NULL, worker, NULL);
    if (r) { fprintf(stderr, "pthread_create failed: %s\n", strerror(r)); return 1; }

    /* Ждём, пока поток «загрязнит» стек и сообщит 'S' */
    if (read(sync_from_worker[0], &ch, 1) != 1) { perror("read S"); return 1; }

    long rss_live = read_rss_kb_statm();
    printf("RSS while worker alive (stack dirtied): %ld KiB\n", rss_live);

    /* Разрешаем потоку выйти (он вернёт NULL) */
    if (write(sync_to_worker[1], "Q", 1) != 1) { perror("write Q"); return 1; }

    /* Ждём подтверждение «E»: поток уже вернулся из start_routine */
    if (read(sync_from_worker[0], &ch, 1) != 1) { perror("read E"); return 1; }

    long rss_after_exit_before_join = read_rss_kb_statm();
    printf("RSS after thread returned (BEFORE join): %ld KiB\n", rss_after_exit_before_join);

    printf("\nПауза перед join — можно проверить /proc/%d/maps | smaps_rollup | pmap.\n", (int)getpid());
    printf("Нажми Enter для pthread_join...\n");
    getchar();

    r = pthread_join(t, NULL);
    if (r) { fprintf(stderr, "pthread_join failed: %s\n", strerror(r)); return 1; }

    long rss_after_join = read_rss_kb_statm();
    printf("RSS after pthread_join: %ld KiB\n", rss_after_join);

    /* Сводка дельт */
    printf("\nΔ alive vs before: %+ld KiB\n", rss_live - rss0);
    printf("Δ after-exit-before-join vs alive: %+ld KiB\n",
           rss_after_exit_before_join - rss_live);
    printf("Δ after-join vs after-exit-before-join: %+ld KiB\n",
           rss_after_join - rss_after_exit_before_join);

    /* Прибираем пайпы */
    close(sync_to_worker[0]); close(sync_to_worker[1]);
    close(sync_from_worker[0]); close(sync_from_worker[1]);
    return 0;
}
