#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
#define NBR_MAX 10

typedef struct {
    int sock;
    struct sockaddr_in addr;
} ClientData;

/* ================= THREAD CLIENT ================= */
void *Chats(void *arg)
{
    ClientData *client = (ClientData *)arg;
    int D = client->sock;

    char pseudoClient[BUF_SIZE];
    char pseudoServeur[] = "Serveur";

    char msg[BUF_SIZE];

    memset(pseudoClient, 0, BUF_SIZE);

    // 1. recevoir pseudo client
    recv(D, pseudoClient, sizeof(pseudoClient), 0);

    printf("Client connecté : %s:%d (%s)\n",
           inet_ntoa(client->addr.sin_addr),
           ntohs(client->addr.sin_port),
           pseudoClient);

    // 2. envoyer pseudo serveur
    send(D, pseudoServeur, strlen(pseudoServeur) + 1, 0);

    // 3. chat simple (serveur répond automatique ou echo)
    while (1)
    {
        memset(msg, 0, BUF_SIZE);

        int r = recv(D, msg, BUF_SIZE, 0);
        if (r <= 0)
            break;

        if (strcmp(msg, "quitter") == 0)
        {
            printf("%s a quitté le chat.\n", pseudoClient);
            break;
        }

        printf("%s : %s\n", pseudoClient, msg);

        // réponse serveur simple (echo)
        char reply[BUF_SIZE];
        snprintf(reply, BUF_SIZE, "OK from server");

        send(D, reply, strlen(reply) + 1, 0);
    }

    close(D);
    free(client);
    return NULL;
}

/* ================= MAIN SERVER ================= */
int main(int argc, char *argv[])
{
    int serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t len = sizeof(clientAddr);

    if (argc != 2)
    {
        printf("Usage: %s PORT\n", argv[0]);
        return 1;
    }

    serverSock = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(atoi(argv[1]));

    if (bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("bind");
        return 1;
    }

    listen(serverSock, NBR_MAX);

    printf("Serveur en attente sur le port %s...\n", argv[1]);

    while (1)
    {
        clientSock = accept(serverSock,
                            (struct sockaddr *)&clientAddr,
                            &len);

        ClientData *data = malloc(sizeof(ClientData));
        data->sock = clientSock;
        data->addr = clientAddr;

        pthread_t thread;
        pthread_create(&thread, NULL, Chats, data);
        pthread_detach(thread);
    }

    close(serverSock);
    return 0;
}
