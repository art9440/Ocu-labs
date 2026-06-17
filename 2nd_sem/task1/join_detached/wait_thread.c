#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

void * work (void * vargs){
    
    sleep(20);
    
    return NULL;
}
int main(){
    pthread_t tid;
    int err;
    
    err = pthread_create(&tid, NULL, work, NULL);
    if (err){
	printf("main: pthread_create() failed: %s\n", strerror(err));
	return -1;
    }
    
    pthread_join(tid, NULL);
    printf("thread is finished!");
    return 0;
}