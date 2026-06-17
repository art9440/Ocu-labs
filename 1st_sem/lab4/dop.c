#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int global = 10;


int main(int argc, char * argv[]){
    scanf("%d", &global);
    sleep(10);
    
    printf("%d \n", global);
    return 0;
}