#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

void * work(void * vargs){
    int count = 0;
    for (;;){
	count++;
	
	if (count > 1000000){
	    pthread_testcancel();
	}
    }
}


int main(){
    pthread_t tid;
    int err;
    err = pthread_create(&tid, NULL, work, NULL);
    
    pthread_cancel(tid);
    
    pthread_join(tid, NULL);
    return 0;
}