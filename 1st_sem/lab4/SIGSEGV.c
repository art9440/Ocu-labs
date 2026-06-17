#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sig) {
    printf("handler: first SIGSEGV caught\n");
    fflush(stdout);

    signal(SIGSEGV, SIG_DFL);

    int *p = NULL;
    *p = 42;
}

int main(void) {
    signal(SIGSEGV, handler);

    int *p = NULL;
    *p = 0;

    printf("After fault\n");
    return 0;
}
