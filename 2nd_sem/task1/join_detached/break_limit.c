#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* worker(void* arg) {
    pause();
    return NULL;
}

int main() {
    pthread_t tid;
    size_t count = 0;
    while (1) {
        if (pthread_create(&tid, NULL, worker, NULL)) {
            perror("pthread_create");
            break;
        }
        count++;
    }
    printf("Created %zu threads\n", count);
    pause();
}
