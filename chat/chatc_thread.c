#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int sock;

char pseudoLocal[50];
char pseudoDistant[50];

/* ================= RECV THREAD ================= */
void *recevoir(void *arg)
{
    char buffer[BUFFER_SIZE];

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        int r = recv(sock, buffer, BUFFER_SIZE, 0);
        if (r <= 0)
        {
            printf("\nConnexion fermée.\n");
            exit(0);
        }

        printf("\n%s : %s\n", pseudoDistant, buffer);
        printf("%s : ", pseudoLocal);
        fflush(stdout);
    }
}

/* ================= SEND THREAD ================= */
void *envoyer(void *arg)
{
    char buffer[BUFFER_SIZE];

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        printf("%s : ", pseudoLocal);
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        send(sock, buffer, strlen(buffer) + 1, 0);

        if (strcmp(buffer, "quitter") == 0)
        {
            printf("Fin du chat.\n");
            exit(0);
        }
    }
}

/* ================= MAIN CLIENT ================= */
int main(int argc, char *argv[])
{
    struct sockaddr_in server;

    if (argc != 3)
    {
        printf("Usage: %s IP PORT\n", argv[0]);
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect");
        return 1;
    }

    printf("Connecté au serveur.\n");

    // pseudo
    printf("Entrer votre pseudo : ");
    fgets(pseudoLocal, 50, stdin);
    pseudoLocal[strcspn(pseudoLocal, "\n")] = '\0';

    // handshake pseudo
    send(sock, pseudoLocal, strlen(pseudoLocal) + 1, 0);
    recv(sock, pseudoDistant, sizeof(pseudoDistant), 0);

    printf("Pseudo distant : %s\n", pseudoDistant);

    pthread_t th1, th2;

    pthread_create(&th1, NULL, recevoir, NULL);
    pthread_create(&th2, NULL, envoyer, NULL);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    close(sock);
    return 0;
}       
