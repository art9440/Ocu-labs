#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

int main() {
    

    // Вызываем нашу обертку вместо стандартного write()
    syscall(SYS_write + 1000, 1, "Hello, world!\n", 14);
    long res = syscall(SYS_write + 500, 1, "Hello, world!\n", 14);
    if (res == -1) {
        perror("syscall failed");
    }

    return 0;
}
