#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        unsigned int cnt = 0;
        while (1) {
            if (write(pipefd[1], &cnt, sizeof(cnt)) != sizeof(cnt)) {
                perror("write");
                break;
            }
            cnt++;
        }
        close(pipefd[1]);
    } else {
        // читатель
        close(pipefd[1]);
        unsigned int prev, cur;
        if (read(pipefd[0], &prev, sizeof(prev)) != sizeof(prev)) exit(0);
        while (read(pipefd[0], &cur, sizeof(cur)) == sizeof(cur)) {
            if (cur != prev + 1) {
                fprintf(stderr,
                    "Разрыв: получили %u, ожидали %u\n", cur, prev + 1);
            }
            prev = cur;
        }
        wait(NULL);
    }
    return 0;
}
