#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>

void* thr(void*){
    pthread_attr_t a;
    void *stack_addr;
    size_t stack_size, guard;

    pthread_getattr_np(pthread_self(), &a);          
    pthread_attr_getstack(&a, &stack_addr, &stack_size);
    pthread_attr_getguardsize(&a, &guard);
    pthread_attr_destroy(&a);

    printf("stack: base=%p, size=%zu bytes, guard=%zu bytes\n",
           stack_addr, stack_size, guard);
    return NULL;
}

int main(void){
    pthread_t t;
    pthread_create(&t, NULL, thr, NULL);
    pthread_join(t, NULL);

    pthread_attr_t a;
    void *addr; size_t sz, guard;
    pthread_getattr_np(pthread_self(), &a);
    pthread_attr_getstack(&a, &addr, &sz);
    pthread_attr_getguardsize(&a, &guard);
    pthread_attr_destroy(&a);
    printf("main stack: base=%p, size=%zu, guard=%zu\n", addr, sz, guard);
}
