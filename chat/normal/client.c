#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void Chats(int socketClient)
{
    char pseudoLocal[50];
    char pseudoDistant[50];

    char messageEnvoye[BUFFER_SIZE];
    char messageRecu[BUFFER_SIZE];

    memset(messageEnvoye, 0, BUFFER_SIZE);
    memset(messageRecu, 0, BUFFER_SIZE);
    memset(pseudoLocal, 0, sizeof(pseudoLocal));
    memset(pseudoDistant, 0, sizeof(pseudoDistant));

    // saisir pseudo local
    printf("Entrer votre pseudonyme : ");
    fgets(pseudoLocal, sizeof(pseudoLocal), stdin);
    pseudoLocal[strcspn(pseudoLocal, "\n")] = '\0';

    // IMPORTANT : ton serveur envoie d'abord son pseudo
    recv(socketClient, pseudoDistant, sizeof(pseudoDistant), 0);

    // puis on envoie le nôtre
    send(socketClient, pseudoLocal, sizeof(pseudoLocal), 0);

    printf("Pseudo distant : %s\n", pseudoDistant);

    while (1)
    {
        memset(messageRecu, 0, BUFFER_SIZE);

        // recevoir message
        recv(socketClient, messageRecu, BUFFER_SIZE, 0);

        if (strcmp(messageRecu, "quitter") == 0)
        {
            printf("%s a quitté la conversation.\n", pseudoDistant);
            break;
        }

        printf("%s : %s\n", pseudoDistant, messageRecu);

        // envoyer message
        memset(messageEnvoye, 0, BUFFER_SIZE);

        printf("%s : ", pseudoLocal);
        fgets(messageEnvoye, BUFFER_SIZE, stdin);
        messageEnvoye[strcspn(messageEnvoye, "\n")] = '\0';

        send(socketClient, messageEnvoye, BUFFER_SIZE, 0);

        if (strcmp(messageEnvoye, "quitter") == 0)
        {
            break;
        }
    }

    close(socketClient);
}

int main(int argc, char *argv[])
{
    int socketClient;
    struct sockaddr_in adresseServeur;

    if (argc != 3)
    {
        printf("Usage : %s <IP_SERVEUR> <PORT>\n", argv[0]);
        return 1;
    }

    socketClient = socket(AF_INET, SOCK_STREAM, 0);
    if (socketClient < 0)
    {
        perror("Erreur socket");
        return 1;
    }

    memset(&adresseServeur, 0, sizeof(adresseServeur));

    adresseServeur.sin_family = AF_INET;
    adresseServeur.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &adresseServeur.sin_addr) <= 0)
    {
        perror("IP invalide");
        return 1;
    }

    if (connect(socketClient,
                (struct sockaddr *)&adresseServeur,
                sizeof(adresseServeur)) < 0)
    {
        perror("Erreur connexion");
        return 1;
    }

    printf("Connecté au serveur.\n");

    Chats(socketClient);

    return 0;
}
