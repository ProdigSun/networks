#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define LISTENQ 10
#define MAXDATASIZE 100

void Bind(int listenfd, struct sockaddr_in *servaddr) {
    // LIGA O SOCKET AO ENDEREÇO E PORTA
    if (bind(listenfd, (struct sockaddr *) servaddr, sizeof((*servaddr))) == -1) {
        perror("bind");
        exit(1);
    }
}

int Listen(int *listenfd) { return ((*listenfd) = socket(AF_INET, SOCK_STREAM, 0)); }

int main(int argc, char **argv) {
    int listenfd, connfd;
    int len;
    struct sockaddr_in servaddr, client;
    FILE *fp;
    char buf[MAXDATASIZE];
    char cmd[] = "echo 'CAT'";
    printf("This file path: %s\n", __FILE__);

    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("Current local time and date: %s", asctime(timeinfo));

    int PORT = atoi(argv[1]);

    if (Listen(&listenfd) == -1) {
        perror("socket");
        exit(1);
    }

    fp = fopen("servidor.txt", "w");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    // BUSCA ENDEREÇO LOCAL DA MÁQUINA NA REDE
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    Bind(listenfd, &servaddr);

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        exit(1);
    }

    // ID DO PROCESSO ATUAL
    int pid;

    for (;;) {
        len = sizeof(client);

        if ((connfd = accept(listenfd, (struct sockaddr *) &client, &len)) == -1) {
            perror("accept");
            exit(1);
        }

        char *client_ip = inet_ntoa(client.sin_addr);
        printf("%s:%d\n", client_ip, ntohs(client.sin_port));

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        fprintf(fp, "Connection time: %s , %s:%d\n", asctime(timeinfo), client_ip, ntohs(client.sin_port));

        pid = fork();
        if (pid == -1) {
            close(connfd);
            continue;

        } else if (pid > 0) {
            close(connfd);
            continue;
        } else if (pid == 0) {
//            sleep(1000000);
            int n = 0;
            int len = 0, maxlen = 100;
            char buffer[MAXDATASIZE];
            char *pbuffer = buffer;
            snprintf(buf, sizeof(buf), "%.24s\r\n", cmd);
            write(connfd, buf, strlen(buf));

           for(;;) {
                n = recv(connfd, pbuffer, maxlen, 0);
                pbuffer += n;
                maxlen -= n;
                len += n;
                printf("received: '%s'\n", buffer);
                fprintf(fp, "received: '%s'\n", buffer);
                if (strncmp(buffer, "", 0) == 0) {
                    printf("\nBroke");
                    break;
                }
            }
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            printf( "\nDisconnection time: %s , %s:%d\n", asctime(timeinfo), client_ip, ntohs(client.sin_port));
            fprintf(fp, "\nDisconnection time: %s , %s:%d\n", asctime(timeinfo), client_ip, ntohs(client.sin_port));

            close(connfd);
        }
        close(connfd);
        fclose(fp);
    }
}
