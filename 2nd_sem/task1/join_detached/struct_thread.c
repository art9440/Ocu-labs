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
    char * str;
} thread_struct;


void * work (void * vargs){
    thread_struct * a = (thread_struct*)vargs;
    printf("num: %d str: %s \n", a->num, a->str);
    free(a->str);
    free(a);
    return NULL;
}


int main(){
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    int err;
    thread_struct * msg = malloc(sizeof * msg);
    msg->num = 10;
    msg->str =strdup( "hello world");
    
    err = pthread_create(&tid, &attr, work, msg);
    if (err){
	printf("main: pthread_create() failed: %s\n", strerror(err));
	free(msg->str);
	free(msg);
	pthread_attr_destroy(&attr);
	return -1;
    }
    pthread_attr_destroy(&attr);
    sleep(50);
    
    
    return 0;
}