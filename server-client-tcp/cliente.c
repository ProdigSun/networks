#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 4096

int main(int argc, char **argv) {
    int sockfd, n;
    char recvline[MAXLINE + 1];
    char error[MAXLINE + 1];
    char local_ip[16];
    unsigned int local_port;
    struct sockaddr_in servaddr, my_addr;

    if (argc != 2) {
        strcpy(error, "uso: ");
        strcat(error, argv[0]);
        strcat(error, " ");
        perror(error);
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }


    bzero(&my_addr, sizeof(my_addr));
    socklen_t len = sizeof(my_addr);

    getsockname(sockfd, (struct sockaddr *) &my_addr, &len);
    inet_ntop(AF_INET, &my_addr.sin_addr, local_ip, sizeof(local_ip));
    local_port = ntohs(my_addr.sin_port);

    printf("(IP %s, PORT %d)\n", local_ip, local_port);

    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            perror("fputs error");
            exit(1);
        }
    }

    if (n < 0) {
        perror("read error");
        exit(1);
    }
    exit(0);
}