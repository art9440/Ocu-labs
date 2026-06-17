#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

int global_var = 100;

void sigchld_handler(int sig) {
    printf("SIGCHLD received (signal %d)\n", sig);
}

int main() {
    int local_var = 200;

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    printf("Parent process:\n");
    printf("Global var address: %p, value: %d\n", (void*)&global_var, global_var);
    printf("Local var address: %p, value: %d\n", (void*)&local_var, local_var);
    printf("Parent PID: %d\n", getpid());

    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (child_pid == 0) {
        printf("\nChild process:\n");
        printf("Child PID: %d, Parent PID: %d\n", getpid(), getppid());

        sleep(1);  
        raise(SIGSTOP); 

        sleep(1);  
        exit(5); 
    } else {
        int status;

        waitpid(child_pid, &status, WUNTRACED);
        if (WIFSTOPPED(status)) {
            printf("Child has been stopped (signal %d)\n", WSTOPSIG(status));
        }

        sleep(2);  

        kill(child_pid, SIGCONT);

        waitpid(child_pid, &status, WCONTINUED);
        if (WIFCONTINUED(status)) {
            printf(" Child has been continued\n");
        }

        waitpid(child_pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Child exited with code %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child terminated by signal %d\n", WTERMSIG(status));
        }
    }

    return 0;
}
