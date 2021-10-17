#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 4096

void Socket(char *const *argv, int *sockfd, struct sockaddr_in *servaddr);

void Connect(int sockfd, struct sockaddr_in *servaddr);

int Read(int sockfd, char *recvline, int *n);

void ExecuteCommand(char **args, const int *pipefd);

void SendCommand(int sockfd, char *path, const int *pipefd);

void validate_args(char *const *argv, char *error);

void ParseCommand(char inputBuffer[], char *args[], int *background) {
    const char s[4] = " \t\n";
    char *token;
    token = strtok(inputBuffer, s);
    int i = 0;
    while (token != NULL) {
        args[i] = token;
        i++;
        printf("%s\n", token);
        token = strtok(NULL, s);
    }
    args[i] = NULL;
}

void reverse(char *x, int begin, int end) {
    char c;

    if (begin >= end)
        return;

    c = *(x + begin);
    *(x + begin) = *(x + end);
    *(x + end) = c;

    reverse(x, ++begin, --end);
}

int main(int argc, char **argv) {
    int sockfd, n;
    char recvline[MAXLINE + 1];
    char error[MAXLINE + 1];
    struct sockaddr_in servaddr, my_addr;

    if (argc != 3) {
        validate_args(argv, error);
    }

    Socket(argv, &sockfd, &servaddr);

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

    Connect(sockfd, &servaddr);

    pid_t pid;

    if ((pid = fork()) == -1)
        perror("fork error");
    else if (pid == 0) {
        bzero(&my_addr, sizeof(my_addr));
        for (;;) {
            printf("READING from server...");

            struct sockaddr_in local_address;
            int addr_size = sizeof(local_address);

            getsockname(sockfd, (struct sockaddr *) &local_address, &addr_size);
            printf("\nSERVER (IP:%s:PORT:%u) > ", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
            printf("\nLOCAL (IP:%s:PORT:%u) > ", inet_ntoa(local_address.sin_addr), ntohs(local_address.sin_port));

            Read(sockfd, recvline, &n);
            recvline[n] = 0;

            if (fputs(recvline, stdout) == EOF) {
                perror("fputs error");
                exit(1);
            }

//            char *args[100];
//            char path[MAXLINE];
//            int pipefd[2];
//
//            ParseCommand(recvline, args, 0);
//
//            if (pipe(pipefd))
//                perror("failed to create a pipe");
//
//            if ((pid = fork()) == -1)
//                perror("fork error");
//            else if (pid == 0) {
//                ExecuteCommand(args, pipefd);
//            } else if (pid > 0) {
//                SendCommand(sockfd, path, pipefd);
//            }
            printf("waiting...");
            if (strncmp(recvline, "", 0) == 0) {
                printf("Broke");
                break;
            }
        }

        reverse(recvline, 0, strlen(recvline) - 1);
        write(sockfd, recvline, strlen(recvline));

        if (n < 0) {
            perror("read error");
            exit(1);
        }
        printf("\nClosing socket...");
        close(sockfd);
        exit(0);
    } else if (pid > 0) {
        while (1) {
            char command[20];
            scanf("%s", command);
            if (strcmp(command, "bye") == 0) {
                close(sockfd);
                return 0;
            }
        }
    }

}

void validate_args(char *const *argv, char *error) {
    strcpy(error, "uso: ");
    strcat(error, argv[0]);
    strcat(error, " ");
    perror(error);
    exit(1);
}

void SendCommand(int sockfd, char *path, const int *pipefd) {
    int lenght;
    close(pipefd[1]);
    memset(path, 0, MAXLINE);
    while (lenght = read(pipefd[0], path, MAXLINE - 1)) {
        printf("Data read so far %s\n", path);
        if (send(sockfd, path, strlen(path), 0) != strlen(path)) {
            perror("Failed");
        }
        fflush(NULL);
        printf("Data sent so far %s\n", path);
        memset(path, 0, MAXLINE);
    }
    close(pipefd[0]);
    close(sockfd);
}

void ExecuteCommand(char **args, const int *pipefd) {
    close(1);
    dup2(pipefd[1], 1);
    close(pipefd[0]);
    close(pipefd[1]);
    execvp(args[0], args);
}

int Read(int sockfd, char *recvline, int *n) { return ((*n) = read(sockfd, recvline, MAXLINE)); }

void Connect(int sockfd, struct sockaddr_in *servaddr) {
    if (connect(sockfd, (struct sockaddr *) servaddr, sizeof((*servaddr))) < 0) {
        perror("connect error");
        exit(1);
    }
}

void Socket(char *const *argv, int *sockfd, struct sockaddr_in *servaddr) {
    if (((*sockfd) = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }

    bzero(servaddr, sizeof((*servaddr)));
    (*servaddr).sin_family = AF_INET;
    (*servaddr).sin_port = htons(atoi(argv[2]));
}
