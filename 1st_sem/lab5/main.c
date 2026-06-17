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

    pid_t pid = getpid();
    printf("Parent PID: %d\n", pid);

    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (child_pid == 0) {
        printf("\nChild process:\n");
        printf("Child PID: %d, Parent PID: %d\n", getpid(), getppid());
        printf("Global var address: %p, value: %d\n", (void*)&global_var, global_var);
        printf("Local var address: %p, value: %d\n", (void*)&local_var, local_var);

        global_var = 500;
        local_var = 600;
        printf("\nAfter modification in child:\n");
        printf("Global var value: %d\n", global_var);
        printf("Local var value: %d\n", local_var);

        exit(5);
    } else {
        sleep(30); // Ждем 30 секунд

        printf("\nParent process after child modification attempt:\n");
        printf("Global var value: %d\n", global_var);
        printf("Local var value: %d\n", local_var);

        int status;
        pid_t waited_pid = wait(&status);
        if (waited_pid == -1) {
            perror("wait failed");
            exit(1);
        }

        if (WIFEXITED(status)) {
            printf("Child exited normally with code %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child terminated by signal %d\n", WTERMSIG(status));
        } else {
            printf("Child exited abnormally\n");
        }
    }

    return 0;
}
