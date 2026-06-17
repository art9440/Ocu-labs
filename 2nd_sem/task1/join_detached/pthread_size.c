#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>

int main(void) {
    unsigned *psz = dlsym(RTLD_DEFAULT, "_thread_db_sizeof_pthread");
    if (psz) printf("sizeof(struct pthread) = %u\n", *psz);
    else     puts("glibc < 2.34 или символ недоступен");
    return 0;
}
