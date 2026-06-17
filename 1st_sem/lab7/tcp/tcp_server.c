#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#define PORT 12346
#define BUFLEN 512

void handle_client(int client_sock) {
    char buffer[BUFLEN];
    int len;
    while ((len = read(client_sock, buffer, BUFLEN)) > 0) {
        write(client_sock, buffer, len);  // echo back
    }
    close(client_sock);
    exit(0);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    signal(SIGCHLD, SIG_IGN); // предотвращает зомби-процессы

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("TCP echo server running on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            close(server_sock);
            handle_client(client_sock);
        } else if (pid > 0) {
            close(client_sock);
        } else {
            perror("fork");
        }
    }

    close(server_sock);
    return 0;
}
