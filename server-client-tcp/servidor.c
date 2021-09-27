#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>

#define LISTENQ 10
#define MAXDATASIZE 100
#define MAXLINE 4096

int main(int argc, char **argv) {
    int listenfd, connfd;
    char local_ip[16];
    unsigned int local_port;
    struct sockaddr_in servaddr, my_addr;
    char buf[MAXDATASIZE];
    time_t ticks;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    // BUSCA ENDEREÇO LOCAL DA MÁQUINA NA REDE
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(0);

    // LIGA O SOCKET AO ENDEREÇO E PORTA
    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        exit(1);
    }

    // ID DO PROCESSO ATUAL
    int pid;

    bzero(&my_addr, sizeof(my_addr));
    socklen_t len = sizeof(my_addr);

    for (;;) {

        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1) {
            perror("accept");
            exit(1);
        }

        getpeername(connfd, (struct sockaddr *) &my_addr, &len);
        inet_ntop(AF_INET, &my_addr.sin_addr, local_ip, sizeof(local_ip));
        local_port = ntohs(my_addr.sin_port);
        printf("(IP %s, PORT %d)\n", local_ip, local_port);

        // DIVIDE PROCESSO COM FORK()
        pid = fork();
        if (pid == -1) {
            // PROCESSO PAI, MAS FILHO NÃO FOI CRIADO
            close(connfd);
            continue;
        } else if (pid > 0) {
            // PROCESSO PAI
            close(connfd);
            ticks = time(NULL);
            continue;
        } else if (pid == 0) {
            // PROCESSO FILHO
            ticks = time(NULL);
            snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
            write(connfd, buf, strlen(buf));
            close(connfd);
        }
    }
    return (0);
}