#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define MAX_CLIENTS 100
#define BUF_SIZE    1024

int  clients[MAX_CLIENTS];
char pseudos[MAX_CLIENTS][50];
int  pseudo_ok[MAX_CLIENTS];
int  client_count = 0;

void broadcast(char *msg, int sender)
{
    for (int i = 0; i < client_count; i++)
        if (clients[i] != sender)
            send(clients[i], msg, strlen(msg) + 1, 0);
}

void remove_client(int index)
{
    close(clients[index]);
    for (int i = index; i < client_count - 1; i++)
    {
        clients[i]   = clients[i + 1];
        pseudo_ok[i] = pseudo_ok[i + 1];
        memcpy(pseudos[i], pseudos[i + 1], 50);
    }
    client_count--;
}

int recv_str(int sock, char *buf, int maxlen)
{
    int i = 0;
    char c;
    while (i < maxlen - 1)
    {
        int r = recv(sock, &c, 1, 0);
        if (r <= 0) return r;
        if (c == '\0') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i + 1;
}

int main(int argc, char *argv[])
{
    int serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t len = sizeof(clientAddr);

    if (argc != 2) { printf("Usage: %s PORT\n", argv[0]); return 1; }

    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; //127.0.0.1 wifi ethernet ...
    serverAddr.sin_port        = htons(atoi(argv[1]));

    bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    listen(serverSock, 10);
    printf("Chat HUB en écoute...\n");

    while (1)
    {
        fd_set readfds; //Liste des sockets surveillées.
        FD_ZERO(&readfds); //vide la liste.
        FD_SET(serverSock, &readfds); //Ajouter serveur Pour surveiller :
        int maxfd = serverSock;

        for (int i = 0; i < client_count; i++)
        {
            FD_SET(clients[i], &readfds);
            if (clients[i] > maxfd) maxfd = clients[i];
        }

        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        /* Nouvelle connexion */
        if (FD_ISSET(serverSock, &readfds))
        {
            clientSock = accept(serverSock, (struct sockaddr *)&clientAddr, &len);
            if (clientSock >= 0 && client_count < MAX_CLIENTS)
            {
                clients[client_count]   = clientSock;
                pseudos[client_count][0] = '\0';
                pseudo_ok[client_count]  = 0;
                client_count++;
            }
        }

        /* Données clients */
        for (int i = 0; i < client_count; i++)
        {
            if (!FD_ISSET(clients[i], &readfds)) continue;

            char buffer[BUF_SIZE];
            int r = recv_str(clients[i], buffer, BUF_SIZE);

            if (r <= 0)
            {
                printf("Client déconnecté : %s\n", pseudos[i]);
                remove_client(i);
                i--;
                continue;
            }

            if (!pseudo_ok[i])
            {
                strncpy(pseudos[i], buffer, 49);
                pseudos[i][49] = '\0';
                pseudo_ok[i] = 1;
                printf("Client connecté : %s\n", pseudos[i]);
                fflush(stdout);
                continue;
            }

            char final[BUF_SIZE];
            snprintf(final, BUF_SIZE, "%s : %s", pseudos[i], buffer);
            printf("Message reçu : %s\n", final);
            fflush(stdout);
            broadcast(final, clients[i]);
        }
    }

    close(serverSock);
    return 0;
}
