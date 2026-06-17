#include <stdio.h>

int main() {
    const char message[] = "Hello, world!\n";
   
    asm volatile (
	"mov $1, %%rax\n"      // SYS_write (номер системного вызова)
        "mov $1, %%rdi\n"      // File descriptor: 1 (stdout)
        "mov %1, %%rsi\n"      // Указатель на строку
        "mov $14, %%rdx\n"     // Длина строки
        "syscall\n"            // Вызов ядра
        :
        : "r"(message)         // Входной параметр
        : "rax", "rdi", "rsi", "rdx"
    );
    return 0;

}
