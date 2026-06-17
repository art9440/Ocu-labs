// udp_receiver.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define SIZE 65535

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }

    char *buffer = malloc(SIZE);
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    int received = recvfrom(sockfd, buffer, SIZE, 0, (struct sockaddr *)&client, &len);
    if (received < 0) perror("recvfrom");
    else {
        printf("Received %d bytes\n", received);
        printf("First few bytes: %.10s\n", buffer);
    }

    close(sockfd);
    free(buffer);
    return 0;
}
