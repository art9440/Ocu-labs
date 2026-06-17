#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <stddef.h>
#include <stdint.h>

struct shm {
    sem_t   sem_empty;  
    sem_t   sem_full;   
    size_t  head, tail;
    uint32_t buf[];
};

int main(void) {
    size_t page_size    = sysconf(_SC_PAGESIZE);
    size_t control_sz   = offsetof(struct shm, buf);
    size_t capacity     = (page_size - control_sz) / sizeof(uint32_t);
    if (capacity < 2) {
        fprintf(stderr, "Ошибка: page size слишком мал для буфера\n");
        exit(1);
    }

    struct shm *sh = mmap(NULL,
                          page_size,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS,
                          -1, 0);
    if (sh == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    sem_init(&sh->sem_empty, 1, capacity);
    sem_init(&sh->sem_full,  1, 0);
    sh->head = sh->tail = 0;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        uint32_t counter = 0;
        for (;;) {
            sem_wait(&sh->sem_empty);
            sh->buf[sh->tail] = counter;
            sh->tail = (sh->tail + 1) % capacity;
            sem_post(&sh->sem_full);
            counter++;
        }
    } else {
        uint32_t expected = 0;
        for (;;) {
            sem_wait(&sh->sem_full);
            uint32_t val = sh->buf[sh->head];
            if (val != expected) {
                fprintf(stderr,
                        "Ошибка последовательности: получили %u, ожидалось %u\n",
                        val, expected);
                expected = val + 1;
            } else {
                expected++;
            }
            sh->head = (sh->head + 1) % capacity;
            sem_post(&sh->sem_empty);
        }
        wait(NULL);
    }

    return 0;
}
