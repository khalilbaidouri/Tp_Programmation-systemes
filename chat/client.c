#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>     // pthread_create, pthread_join
#include <arpa/inet.h>   // inet_pton, htons, sockaddr_in

#define BUF_SIZE 1024

int  sock;       // socket global (partagé entre les deux threads)
char pseudo[50]; // pseudo global (partagé entre les deux threads)

/* Thread qui écoute en permanence les messages venant du serveur */
void *recv_thread(void *arg)
{
    char buffer[BUF_SIZE];
    while (1)
    {
        memset(buffer, 0, BUF_SIZE);           // vide le buffer avant chaque lecture
        int r = recv(sock, buffer, BUF_SIZE, 0); // bloque jusqu'à réception d'un message
        if (r <= 0)
        {
            printf("\nServeur déconnecté\n");
            exit(0); // ferme tout le programme si le serveur coupe
        }
        // \r revient au début de la ligne pour effacer l'invite de saisie
        // puis affiche le message et réaffiche l'invite
        printf("\r%s\n%s : ", buffer, pseudo);
        fflush(stdout); // force l'affichage immédiat
    }
}

/* Thread qui lit le clavier et envoie les messages au serveur */
void *send_thread(void *arg)
{
    char msg[BUF_SIZE];
    while (1)
    {
        printf("%s : ", pseudo);  // affiche l'invite de saisie
        fflush(stdout);

        if (fgets(msg, BUF_SIZE, stdin) == NULL) // lit une ligne du clavier
            break;

        msg[strcspn(msg, "\n")] = '\0'; // remplace le \n de fgets par \0

        if (strlen(msg) == 0) // ignore si l'utilisateur appuie juste sur Entrée
            continue;

        send(sock, msg, strlen(msg) + 1, 0); // envoie le message + le \0 final

        if (strcmp(msg, "quitter") == 0) // commande spéciale pour quitter
            exit(0);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server; // structure qui contient l'IP et le port du serveur

    if (argc != 3)
    {
        printf("Usage: %s IP PORT\n", argv[0]);
        return 1;
    }

    // crée un socket TCP
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    server.sin_family = AF_INET;               // IPv4
    server.sin_port   = htons(atoi(argv[2]));  // port en big-endian réseau
    inet_pton(AF_INET, argv[1], &server.sin_addr); // convertit "192.168.1.1" en binaire

    // tente de se connecter au serveur
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect");
        return 1;
    }

    // demande le pseudo à l'utilisateur
    printf("Pseudo : ");
    fflush(stdout);
    fgets(pseudo, 50, stdin);
    pseudo[strcspn(pseudo, "\n")] = '\0'; // supprime le \n

    // envoie le pseudo au serveur (premier message = pseudo)
    send(sock, pseudo, strlen(pseudo) + 1, 0);

    // crée les deux threads en parallèle
    pthread_t t1, t2;
    pthread_create(&t1, NULL, recv_thread, NULL); // thread réception
    pthread_create(&t2, NULL, send_thread, NULL); // thread envoi

    // attend que les threads se terminent (normalement jamais, sauf exit())
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    close(sock); // ferme le socket proprement
    return 0;
}