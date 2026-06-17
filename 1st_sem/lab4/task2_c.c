#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#define _GNU_SOURCE

#define STACK_ALLOC_SIZE 4096
#define HEAP_ITERATIONS 10
#define HEAP_ALLOC_SIZE 8192
#define PAGE_SIZE 4096
#define MMAP_PAGES 10

void sigsegv_handler(int sig) {
    printf("Caught SIGSEGV: Segmentation Fault occurred\n");
    exit(1);
}


void stack_bomb(int depth) {
    char stack_buf[STACK_ALLOC_SIZE];
    memset(stack_buf, 1, sizeof(stack_buf));
    printf("depth: %d. address: %p\n", depth, (void*)stack_buf);
    usleep(100 * 1000);
    stack_bomb(depth + 1);
}


int main(int argc, char * argv[]){
    printf("PID: %d\n", getpid());
    
    sleep(15);
    
    //stack_bomb(1);
    
    char** heap_buffer = malloc(HEAP_ITERATIONS * sizeof(char *));
    for (size_t i = 0; i < HEAP_ITERATIONS; i++){
	heap_buffer[i] = malloc(HEAP_ALLOC_SIZE);
	memset(heap_buffer[i], 0, HEAP_ALLOC_SIZE);
	//printf("Allocated heap buffer[%d]: %p\n", i, (void *)heap_buffer[i]);
	sleep(1);
    }
    
    for (int i = 0; i < HEAP_ITERATIONS; ++i) {
	free(heap_buffer[i]);
    }
    free(heap_buffer);
    
     signal(SIGSEGV, sigsegv_handler);
    
    //printf("Mapping anonymous memory with mmap...\n");
  void *region = mmap(NULL, MMAP_PAGES * PAGE_SIZE,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (region == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }
 // printf("Mapped memory at %p (size = %d pages)\n", region, MMAP_PAGES);
    
  
  //printf("Mapped memory set. Check /proc/%d/maps\n", getpid());
  sleep(10);
  
  strcpy(region, "Hello, mmap!");

    
    //printf("Initial content in region: %s\n", (char *)region);

    
    if (mprotect(region, MMAP_PAGES * PAGE_SIZE, PROT_WRITE) == -1) {
        perror("mprotect: read access");
        return 1;
    }

    
    //printf("Trying to read after disabling read access...\n");
    //printf("%s\n", (char *)region); // SIGSEGV

    if (mprotect(region, MMAP_PAGES * PAGE_SIZE, PROT_READ) == -1) {
        perror("mprotect: write access");
        return 1;
    }

    
    //printf("Trying to write after disabling write access...\n");
    strcpy((char *)region, "New content!"); // SIGSEGV
    
     void *start = region + (PAGE_SIZE * 3); 
    if (munmap(start, PAGE_SIZE * 3) == -1) {
        perror("munmap failed");
        exit(1);
    }

    
    munmap(region, MMAP_PAGES * PAGE_SIZE);
    
    
    return 0;
}