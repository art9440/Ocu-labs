#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char * argv[]){

    // printf("PID: %d\n", getpid());
    
    sleep(1);
    
    execv(argv[0], argv);
    
    printf("Hello World!\n");
    
    perror("execv failed");
    return 1;
}