#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

void * work (void * vargs){
    const char *msg = "hello world";
    char * copy = strdup(msg);
    if (!copy) return NULL;
	
    return copy;
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
	printf("Returned message: %s \n", (char*)ret);
	free(ret);
    }
    
    return 0;
}