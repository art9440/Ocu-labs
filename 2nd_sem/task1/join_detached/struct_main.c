#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct {
    int num;
    char *str;
} thread_struct;

void *work(void *vargs) {
    sleep(100);
    thread_struct *a = (thread_struct*)vargs;
    printf("num: %d str: %s\n", a->num, a->str);
    // Ничего не free: данные не из кучи
    return NULL;
}

int main(void) {
    pthread_t tid;
    int err;

    // Структура и строковый буфер на стеке главного потока
    thread_struct msg;
    char buf[64];
    msg.num = 10;
    snprintf(buf, sizeof(buf), "hello world");
    msg.str = buf;

    err = pthread_create(&tid, NULL, work, &msg);
    if (err) {
        fprintf(stderr, "main: pthread_create() failed: %s\n", strerror(err));
        return -1;
    }
    sleep(50);
    // Ждём завершения, чтобы стековые данные оставались валидны
    pthread_exit(NULL);
    if (err) {
        fprintf(stderr, "main: pthread_join() failed: %s\n", strerror(err));
        return -1;
    }

    return 0;
}
