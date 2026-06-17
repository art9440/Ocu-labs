#include <stdio.h>

int main() {
    printf("Alignment of char: %zu\n", _Alignof(char));
    printf("Alignment of int: %zu\n", _Alignof(int));
    printf("Alignment of double: %zu\n", _Alignof(double));
    printf("Alignment of void*: %zu\n", _Alignof(void*));
    printf("Aligment of unsigned long long int: %lld\n", _Alignof(unsigned long long int));
    printf("Alignment of size_t: %zu\n", _Alignof(max_align_t));
    printf("sizeof(size_t): %d\n", sizeof(size_t));
    return 0;
}
