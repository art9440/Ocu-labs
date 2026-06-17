#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFLEN 512

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFLEN];
    socklen_t addr_len = sizeof(client_addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP echo server is running on port %d\n", PORT);

    while (1) {
        int len = recvfrom(sockfd, buffer, BUFLEN, 0,
                           (struct sockaddr *)&client_addr, &addr_len);
        if (len < 0) {
            perror("recvfrom");
            continue;
        }

        buffer[len] = '\0';
        printf("Received from client: %s\n", buffer);

        sendto(sockfd, buffer, len, 0,
               (struct sockaddr *)&client_addr, addr_len);
    }

    close(sockfd);
    return 0;
}
