#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>   // htons, inet_pton, sockaddr_in
#include <sys/select.h>  // select, fd_set, FD_SET, FD_ZERO, FD_ISSET

#define MAX_CLIENTS 100  // maximum de clients simultanés
#define BUF_SIZE    1024 // taille max d'un message

/* --- Données globales (partagées dans tout le fichier) --- */
int  clients[MAX_CLIENTS];      // numéros de sockets des clients
char pseudos[MAX_CLIENTS][50];  // pseudo de chaque client (max 49 chars)
int  pseudo_ok[MAX_CLIENTS];    // 0 = pseudo pas encore reçu, 1 = ok
int  client_count = 0;          // nombre de clients actuellement connectés

/* Envoie msg à tous les clients SAUF l'expéditeur (sender) */
void broadcast(char *msg, int sender)
{
    for (int i = 0; i < client_count; i++)
        if (clients[i] != sender)                      // on saute l'expéditeur
            send(clients[i], msg, strlen(msg) + 1, 0); // +1 pour envoyer le \0
}

/* Supprime le client à l'index donné et compacte les tableaux */
void remove_client(int index)
{
    close(clients[index]); // ferme la connexion réseau

    // décale tout ce qui est après index vers la gauche (comble le trou)
    for (int i = index; i < client_count - 1; i++)
    {
        clients[i]   = clients[i + 1];
        pseudo_ok[i] = pseudo_ok[i + 1];
        memcpy(pseudos[i], pseudos[i + 1], 50); // copie 50 octets du pseudo
    }
    client_count--; // un client de moins
}

/* Lit un message depuis un socket octet par octet jusqu'au \0
   Retourne le nombre d'octets lus, 0 ou négatif si déconnexion */
int recv_str(int sock, char *buf, int maxlen)
{
    int i = 0;
    char c;
    while (i < maxlen - 1)
    {
        int r = recv(sock, &c, 1, 0); // lit 1 seul octet
        if (r <= 0) return r;         // 0 = déconnexion, -1 = erreur
        if (c == '\0') break;         // fin du message détectée
        buf[i++] = c;                 // ajoute l'octet au buffer
    }
    buf[i] = '\0';  // termine la chaîne proprement
    return i + 1;
}

int main(int argc, char *argv[])
{
    int serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr; // structures d'adresse IP
    socklen_t len = sizeof(clientAddr);        // taille de l'adresse client

    if (argc != 2) { printf("Usage: %s PORT\n", argv[0]); return 1; }

    // crée un socket TCP (AF_INET=IPv4, SOCK_STREAM=TCP)
    serverSock = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    // SO_REUSEADDR : permet de réutiliser le port immédiatement après arrêt
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serverAddr, 0, sizeof(serverAddr));       // met tout à zéro
    serverAddr.sin_family      = AF_INET;             // IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY;          // écoute sur toutes les interfaces
    serverAddr.sin_port        = htons(atoi(argv[1]));// convertit le port en big-endian réseau

    // attache le socket à l'adresse et au port
    bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    // met le socket en écoute, max 10 connexions en attente
    listen(serverSock, 10);
    printf("Chat HUB en écoute...\n");

    while (1) // boucle infinie du serveur
    {
        fd_set readfds;
        FD_ZERO(&readfds);              // vide le set (obligatoire à chaque tour)
        FD_SET(serverSock, &readfds);   // surveille le socket serveur (nouvelles connexions)
        int maxfd = serverSock;

        // ajoute chaque client au set
        for (int i = 0; i < client_count; i++)
        {
            FD_SET(clients[i], &readfds);
            if (clients[i] > maxfd) maxfd = clients[i]; // garde le plus grand fd
        }

        // BLOQUE ici jusqu'à ce qu'un socket soit actif
        // maxfd+1 car select() attend le nombre de fd à surveiller
        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        /* --- Nouvelle connexion entrante ? --- */
        if (FD_ISSET(serverSock, &readfds))
        {
            // accept() crée un nouveau socket dédié à ce client
            clientSock = accept(serverSock, (struct sockaddr *)&clientAddr, &len);
            if (clientSock >= 0 && client_count < MAX_CLIENTS)
            {
                clients[client_count]    = clientSock; // enregistre le socket
                pseudos[client_count][0] = '\0';       // pseudo vide pour l'instant
                pseudo_ok[client_count]  = 0;          // pseudo pas encore reçu
                client_count++;
            }
        }

        /* --- Message d'un client existant ? --- */
        for (int i = 0; i < client_count; i++)
        {
            // ce client n'a pas envoyé de données → on passe
            if (!FD_ISSET(clients[i], &readfds)) continue;

            char buffer[BUF_SIZE];
            int r = recv_str(clients[i], buffer, BUF_SIZE);

            if (r <= 0) // déconnexion ou erreur
            {
                printf("Client déconnecté : %s\n", pseudos[i]);
                remove_client(i);
                i--; // recule l'index car le tableau a été décalé
                continue;
            }

            if (!pseudo_ok[i]) // premier message reçu = c'est le pseudo
            {
                strncpy(pseudos[i], buffer, 49); // copie max 49 chars
                pseudos[i][49] = '\0';            // sécurise la fin de chaîne
                pseudo_ok[i] = 1;                 // pseudo validé
                printf("Client connecté : %s\n", pseudos[i]);
                fflush(stdout); // force l'affichage immédiat
                continue;
            }

            // message normal : formate "pseudo : message" et diffuse
            char final[BUF_SIZE];
            snprintf(final, BUF_SIZE, "%s : %s", pseudos[i], buffer);
            printf("Message reçu : %s\n", final);
            fflush(stdout);
            broadcast(final, clients[i]); // envoie à tout le monde sauf l'expéditeur
        }
    }

    close(serverSock); // jamais atteint mais bonne pratique
    return 0;
}