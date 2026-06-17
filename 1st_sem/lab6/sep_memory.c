#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>

#define PAGE_ENTRIES (sysconf(_SC_PAGESIZE) / sizeof(unsigned int))

int main() {
    // 1. mmap: анонимная, разделяемая
    unsigned int *buf = mmap(NULL,
                             PAGE_ENTRIES * sizeof(unsigned int),
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS,
                             -1, 0);
    if (buf == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    
     if (pid == 0) {
        // ребёнок — писатель
        unsigned int cnt = 0;
        size_t idx = 0;
        while (1) {
            buf[idx] = cnt;
            cnt++;
            idx = (idx + 1) % PAGE_ENTRIES;
        }
    }
    else {
        // родитель — читатель
        unsigned int prev = buf[0];
        size_t idx = 1;
        while (1) {
            unsigned int cur = buf[idx];
            if (cur != prev + 1) {
                fprintf(stderr,
                    "Разрыв последовательности: buf[%zu]=%u, ожидалось %u\n",
                    idx, cur, prev + 1);
            }
            prev = cur;
            idx = (idx + 1) % PAGE_ENTRIES;
        }
    }
    return 0;
}
