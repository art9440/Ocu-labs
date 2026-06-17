#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static void on_sigint(int signo) {
    const char msg[] = "t2: caught SIGINT via handler\n";
    (void)signo;
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

static void* t1_block_all(void* arg) {
    (void)arg;
    sigset_t all; sigfillset(&all);
    pthread_sigmask(SIG_SETMASK, &all, NULL);
    for (;;) sleep(1);
    return NULL;
}

static void* t2_handle_sigint(void* arg) {
    (void)arg;
    sigset_t set; sigemptyset(&set); sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    for (;;) sleep(1);
    return NULL;
}

static void* t3_sigwait_quit(void* arg) {
    (void)arg;
    sigset_t set; sigemptyset(&set); sigaddset(&set, SIGQUIT);
    int sig;
    for (;;) {
        int r = sigwait(&set, &sig);
        if (r == 0 && sig == SIGQUIT) {
            write(STDOUT_FILENO, "t3: got SIGQUIT via sigwait\n", 29);
        }
    }
    return NULL;
}

int main(){
    sigset_t all; sigfillset(&all);
    pthread_sigmask(SIG_SETMASK, &all, NULL);
    
     struct sigaction sa; memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction"); return 1;
    }

    pthread_t th1, th2, th3;
    if (pthread_create(&th1, NULL, t1_block_all,   NULL)) { perror("t1"); return 1; }
    if (pthread_create(&th2, NULL, t2_handle_sigint, NULL)) { perror("t2"); return 1; }
    if (pthread_create(&th3, NULL, t3_sigwait_quit, NULL)) { perror("t3"); return 1; }


    for (;;) sleep(10);
    return 0;
}