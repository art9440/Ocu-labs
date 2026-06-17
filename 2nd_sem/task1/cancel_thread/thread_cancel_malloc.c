#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

void * work(void * vargs){
    char * str = malloc(sizeof(char)  * 11);
    pthread_cleanup_push(free, str);
    str = "hello world";
    for (;;){
	printf("str : %s \n", str);
    }
    pthread_cleanup_pop(1);
    return NULL;
}


int main(){
    pthread_t tid;
    int err;
    err = pthread_create(&tid, NULL, work, NULL);
    
    pthread_cancel(tid);
    
    pthread_join(tid, NULL);
    return 0;
}