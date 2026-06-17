#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* worker(void* arg) {
    // поток сразу завершается
    return NULL;
}

int main(void) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setguardsize(&attr, 0); // без guard-страниц (как у тебя)

    const int N = 1000;
    for (int i = 0; i < N; i++) {
        pthread_t tid;
        if (pthread_create(&tid, &attr, worker, NULL) != 0) {
            perror("pthread_create");
            fprintf(stderr, "Created %d threads before failure\n", i);
            exit(1);
        }
    }

    printf("Created %d dead threads. Sleeping 100s...\n", N);
    sleep(100); // время для измерений
    return 0;
}
