#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/syscall.h>

static pid_t gettid_wrap(void){ return (pid_t)syscall(SYS_gettid); }

static void on_sigint(int signo) {
    (void)signo;
    char buf[128];
    int n = snprintf(buf, sizeof(buf),
                     "handler: SIGINT in TID=%d\n", (int)gettid_wrap());
    write(STDOUT_FILENO, buf, n);
}

static void* t1_block_all(void* arg) {
    (void)arg;
    sigset_t all; sigfillset(&all);
    pthread_sigmask(SIG_SETMASK, &all, NULL);
    for (;;) pause();
}

static void* t2_handle_sigint(void* arg) {
    (void)arg;
    sigset_t set; sigemptyset(&set); sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    // покажем TID этого потока
    dprintf(STDOUT_FILENO, "t2 thread TID=%d (SIGINT unblocked)\n", (int)gettid_wrap());

    for (;;) pause();
}

static void* t3_sigwait_quit(void* arg) {
    (void)arg;
    sigset_t set; sigemptyset(&set); sigaddset(&set, SIGQUIT);

    for (;;) {
        int sig;
        int r = sigwait(&set, &sig);
        if (r == 0 && sig == SIGQUIT) {
            dprintf(STDOUT_FILENO, "t3: got SIGQUIT via sigwait in TID=%d\n",
                    (int)gettid_wrap());
        }
    }
}

int main(){
    sigset_t all; sigfillset(&all);
    pthread_sigmask(SIG_SETMASK, &all, NULL);

    struct sigaction sa; memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; 
    if (sigaction(SIGINT, &sa, NULL) == -1) { perror("sigaction"); return 1; }

    pthread_t th1, th2, th3;
    if (pthread_create(&th1, NULL, t1_block_all,     NULL)) { perror("t1"); return 1; }
    if (pthread_create(&th2, NULL, t2_handle_sigint, NULL)) { perror("t2"); return 1; }
    if (pthread_create(&th3, NULL, t3_sigwait_quit,  NULL)) { perror("t3"); return 1; }

    sleep(1);
    
    for (;;){
	pause();
    }


}
