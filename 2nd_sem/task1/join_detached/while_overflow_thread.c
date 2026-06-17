#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

void * work (void * vargs){
    printf("gettid: %d\n", gettid());
    return NULL;
}


int main(){
    pthread_t tid;
    int err;
    while(1){
    err = pthread_create(&tid, NULL, work, NULL);
    if (err){
	printf("main: pthread_create() failed: %s\n", strerror(err));
	return -1;
    }
    }
    
    return 0;
}