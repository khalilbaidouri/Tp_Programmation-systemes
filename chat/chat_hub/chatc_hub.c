#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int  sock;
char pseudo[50];

void *recv_thread(void *arg)
{
    char buffer[BUF_SIZE];
    while (1)
    {
        memset(buffer, 0, BUF_SIZE);
        int r = recv(sock, buffer, BUF_SIZE, 0);
        if (r <= 0)
        {
            printf("\nServeur déconnecté\n");
            exit(0);
        }
        printf("\r%s\n%s : ", buffer, pseudo);
        fflush(stdout);
    }
}

void *send_thread(void *arg)
{
    char msg[BUF_SIZE];
    while (1)
    {
        printf("%s : ", pseudo);
        fflush(stdout);

        if (fgets(msg, BUF_SIZE, stdin) == NULL)
            break;
        msg[strcspn(msg, "\n")] = '\0';

        if (strlen(msg) == 0)
            continue;

        send(sock, msg, strlen(msg) + 1, 0);

        if (strcmp(msg, "quitter") == 0)
            exit(0);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server;

    if (argc != 3)
    {
        printf("Usage: %s IP PORT\n", argv[0]);
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    server.sin_family = AF_INET;
    server.sin_port   = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect");
        return 1;
    }

    printf("Pseudo : ");
    fflush(stdout);
    fgets(pseudo, 50, stdin);
    pseudo[strcspn(pseudo, "\n")] = '\0';

    send(sock, pseudo, strlen(pseudo) + 1, 0);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, recv_thread, NULL);
    pthread_create(&t2, NULL, send_thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    close(sock);
    return 0;
}
