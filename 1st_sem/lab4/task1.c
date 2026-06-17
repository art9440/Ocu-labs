#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int glob_1 = 5;
int glob_2;
const int glob_3 = 10;


void task_e(){
    char * buffer = (char*)malloc(100 * sizeof(char));
    
    if (!buffer){
	perror("malloc");
	return;
    }
    
    strcpy(buffer, "Hello World!");
    printf("buffer: %s\n", buffer);
    free(buffer);
    
    printf("buffer after free: %s\n", buffer);
    
    char * buffer1 = (char*)malloc(100 * sizeof(char));
    
    if (!buffer){
	perror("malloc");
	return;
    }
    
    strcpy(buffer1, "Hello Universe!");
    printf("buffer1: %s\n", buffer1);
    
    char *mid_ptr = buffer1 + 50;
    
    free(mid_ptr);
    
    printf("buffer1 after mid free: %s\n", buffer1);
}

void task_g(){
     const char* val = getenv("MYVAR");
  printf("Initial MYVAR: %s\n", val ? val : "not set");

  setenv("MYVAR", "new_value", 1);
  
  val = getenv("MYVAR");
  printf("Updated MYVAR: %s\n", val);
}


void task_d(){
    int loc;
    loc = 100;
    int *p;
    p = &loc;
    printf("loc for task_d: %p\n", p);
}

void task_a(){
    int x = 10;
    int *p_1;
    p_1 = &x;
    printf("loc_var: %p \n", p_1);
    
    int *p_2;
    p_2 = &glob_1;
    printf("glob_var_in: %p \n", p_2);
    
    int *p_3;
    p_3 = &glob_2;
    printf("glob_var_noin: %p \n", p_3);
    
    static int y = 0;
    int *p_4;
    p_4 = &y;
    printf("static_var: %p \n", p_4);
    
    const int z = 0;
    int *p_5;
    p_5 = &z;
    printf("const_var: %p \n", p_5);
    
    int *p_6;
    p_6 = &glob_3;
    printf("const_glob_var: %p \n", p_6);
}


int main(int argc, char * argv[]){
    if(strcmp(argv[0], "./task_a") == 0){
	task_a();
    }
    else if(strcmp(argv[0], "./task_d") == 0){
	task_d();
    }
    else if(strcmp(argv[0], "./task_e") == 0){
	task_e();
    }
    else if(strcmp(argv[0], "./task_g") == 0){
	task_g();
    }
    
    pid_t pid = getpid();

    // Выводим PID
    printf("PID текущего процесса: %d\n", pid);
    sleep(15);
    
    return 0;
}