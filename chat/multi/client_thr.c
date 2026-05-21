#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUF 1024

int sock;
char pseudo[50];

void *recv_thread(void *arg)
{
    char buffer[BUF];

    while (1)
    {
        memset(buffer, 0, BUF);

        if (recv(sock, buffer, BUF, 0) <= 0)
        {
            printf("\nServeur fermé\n");
            exit(0);
        }

        printf("\n%s\n", buffer);
        printf("%s : ", pseudo);
        fflush(stdout);
    }
}

void *send_thread(void *arg)
{
    char buffer[BUF];
    char final[BUF];

    while (1)
    {
        printf("%s : ", pseudo);
        fgets(buffer, BUF, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        snprintf(final, BUF, "%s : %s", pseudo, buffer);

        send(sock, final, strlen(final) + 1, 0);

        if (strcmp(buffer, "quitter") == 0)
            exit(0);
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server.sin_addr);

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    printf("Pseudo : ");
    fgets(pseudo, 50, stdin);
    pseudo[strcspn(pseudo, "\n")] = '\0';

    pthread_t t1, t2;

    pthread_create(&t1, NULL, recv_thread, NULL);
    pthread_create(&t2, NULL, send_thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    close(sock);
    return 0;
}
