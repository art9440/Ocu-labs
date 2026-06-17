#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

int glob = 100;


struct thread_arg {
    int idx;
    pthread_t *self_ids;
};

void *mythread(void *arg) {
	int loc = 10;
	static int loc_static = 20;
	const int loc_const = 1;
	struct thread_arg *a = (struct thread_arg*)arg;
	
	pthread_t self = pthread_self();
	
	 a->self_ids[a->idx] = self;
	 
	printf("mythread [%d %d %d]: Hello from mythread! idx = %d \n", getpid(), getppid(), gettid(), a->idx);
	printf("glob: %p, loc: %p, loc_static: %p, loc_const: %p. idx = %d \n",(void*) &glob,(void*) &loc,(void*) &loc_static,(void*) &loc_const, a->idx);
	
	
	return NULL;
}

int main() {
	
	pthread_t tids[5];
	pthread_t self_ids[5];
	struct thread_arg args[5];
	int err;

	printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());
	for (int i = 0; i < 5; i++){
	    args[i].idx = i;
	    args[i].self_ids = self_ids;
	    
	    err = pthread_create(&tids[i], NULL, mythread, &args[i]);
	    if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
		return -1;
	    }
	}
	
	sleep(100);
	
	
	for (int i = 0; i < 5; i++){
	    pthread_join(tids[i], NULL);
	}
	
	for (int i = 0; i < 5; i++){
	    int eq = pthread_equal(tids[i], self_ids[i]);
	    
	    printf("check #%d: pthread_create ID %s pthread_self()\n", i, eq ? "==" : "!=");
	}
	
	return 0;
}

