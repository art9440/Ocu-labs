#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

static pid_t gettid_() { return (pid_t)syscall(SYS_gettid); }

static void print_stacks_by_thread(const char *tag) {
    printf("\n==== %s ====\n", tag);

    // перечислим все TID процесса
    char taskdir[64];
    snprintf(taskdir, sizeof(taskdir), "/proc/self/task");
    DIR *d = opendir(taskdir);
    if (!d) { perror("opendir /proc/self/task"); return; }

    struct dirent *de;
    while ((de = readdir(d))) {
        if (de->d_name[0] == '.') continue;

        char maps[128];
        snprintf(maps, sizeof(maps), "/proc/self/task/%s/maps", de->d_name);
        FILE *f = fopen(maps, "r");
        if (!f) continue;

        int header = 0;
        char line[1024];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "[stack")) {
                if (!header) {
                    printf("-- TID %s --\n", de->d_name);
                    header = 1;
                }
                fputs(line, stdout);
            }
        }
        fclose(f);
    }
    closedir(d);

    // общий maps процесса: покажем только строки со стеком
    printf("-- /proc/self/maps (process-wide) --\n");
    FILE *fm = fopen("/proc/self/maps", "r");
    if (fm) {
        char line[1024];
        while (fgets(line, sizeof(line), fm)) {
            if (strstr(line, "[stack")) fputs(line, stdout);
        }
        fclose(fm);
    } else {
        perror("open /proc/self/maps");
    }
}

static int path_exists(const char *p) {
    struct stat st;
    return stat(p, &st) == 0;
}

static void* worker(void *arg) {
    (void)arg;
    pid_t pid  = getpid();
    pid_t tid  = gettid_();
    printf("[worker] pid=%d tid=%d started\n", pid, tid);

    // даём main распечатать "до" и уйти через pthread_exit
    sleep(2);

    // после ухода main: лидер (TID==PID) должен исчезнуть
    char leader_maps[128];
    snprintf(leader_maps, sizeof(leader_maps), "/proc/self/task/%d/maps", pid);

    print_stacks_by_thread("AFTER main pthread_exit");

    printf("[worker] check leader task dir: %s -> %s\n",
           leader_maps, path_exists(leader_maps) ? "STILL EXISTS (unexpected)" : "MISSING (expected)");

    return NULL;
}

int main(void) {
    pid_t pid = getpid();
    pid_t tid = gettid_();
    printf("[main]  pid=%d tid=%d\n", pid, tid);

    pthread_t th;
    if (pthread_create(&th, NULL, worker, NULL)) {
        perror("pthread_create");
        return 1;
    }

    // маленькая пауза: чтобы worker успел стартовать
    usleep(200*1000);

    // снимок ДО: увидим [stack] у лидера и [stack:<tid_worker>] в /proc/self/maps
    print_stacks_by_thread("BEFORE main pthread_exit");

    // уходим из главного потока, процесс оставляем жить на worker
    pthread_exit(NULL);

    // не достигнется
    return 0;
}
