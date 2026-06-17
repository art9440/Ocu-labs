#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define SIZE 65507

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);

    char *buffer = malloc(SIZE);
    memset(buffer, 'A', SIZE);

    int sent = sendto(sockfd, buffer, SIZE, 0, (struct sockaddr *)&serv, sizeof(serv));
    if (sent < 0) perror("sendto");
    else printf("Sent %d bytes\n", sent);

    close(sockfd);
    free(buffer);
    return 0;
}
