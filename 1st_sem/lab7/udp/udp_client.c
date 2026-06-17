#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUFLEN 512

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFLEN];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    while (1) {
        printf("Enter message: ");
        if (fgets(buffer, BUFLEN, stdin) == NULL) {
	    printf("EOF received. Exiting loop.\n");
	    clearerr(stdin);
            continue;
	}
        buffer[strcspn(buffer, "\n")] = 0;

        sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr));

        int len = recvfrom(sockfd, buffer, BUFLEN, 0, NULL, NULL);
        buffer[len] = '\0';
        printf("Echo from server: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
