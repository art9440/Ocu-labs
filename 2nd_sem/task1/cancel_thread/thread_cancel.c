#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

void * work(void * vargs){
    for(;;){
	printf("Hello World!");
    }
    
    return NULL;
}


int main(){
    pthread_t tid;
    int err;
    err = pthread_create(&tid, NULL, work, NULL);
    
    
    pthread_cancel(tid);
    return 0;
}