#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int global_var = 100;

int main() {
    int local_var = 200;

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
        exit(5);  
    } else {
        printf("\nParent process: sleeping for 30 seconds...\n");
        sleep(30);

        printf("\nParent process: now calling wait()...\n");
        int status;
        wait(&status);

        if (WIFEXITED(status)) {
            printf("Child exited normally with code %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child terminated by signal %d\n", WTERMSIG(status));
        }
    }

    return 0;
}
