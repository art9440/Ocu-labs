#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

void * work (void * vargs){
    int * res = malloc(sizeof * res);
    if (!res) return NULL;
	
    *res = 42;
    return res;
}


int main(){
    pthread_t tid;
    int err;
    
    err = pthread_create(&tid, NULL, work, NULL);
    if (err){
	printf("main: pthread_create() failed: %s\n", strerror(err));
	return -1;
    }
    void * ret = NULL;
    pthread_join(tid, &ret);
    printf("thread is finished!\n");
    if (ret){
	int value = *(int*)ret;
	printf("Returned value: %d \n", value);
	free(ret);
    }
    
    return 0;
}