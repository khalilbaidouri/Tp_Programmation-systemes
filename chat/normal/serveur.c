#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
#define NBR_MAX  10

/* ============================================================
   Fonction Chats(Entier D)  —  Image 2 (tableau)
   ============================================================ */
void Chats(int D) {
    char psl[BUF_SIZE];  // pseudo local
    char psd[BUF_SIZE];  // pseudo distant
    char mr[BUF_SIZE];   // message reçu
    char me[BUF_SIZE];   // message envoyé

    /* -- Début -- */

    // Initialiser(mr, me)
    memset(mr, 0, BUF_SIZE);
    memset(me, 0, BUF_SIZE);

    // Initialiser(psl, psd)
    memset(psl, 0, BUF_SIZE);
    memset(psd, 0, BUF_SIZE);

    // Afficher("Saisir votre pseudo nom :")
    printf("Saisir votre pseudo nom : ");
    fgets(psl, BUF_SIZE, stdin);
    psl[strcspn(psl, "\n")] = '\0';

    // Lire(psl)  — déjà fait avec fgets ci-dessus

    // Send(D, psl)  — envoyer notre pseudo au distant
    send(D, psl, strlen(psl) + 1, 0);

    // Recv(D, psd)  — recevoir le pseudo du distant
    recv(D, psd, BUF_SIZE, 0);

    // Afficher("pseudo nom distant est : psd")
    printf("Pseudo nom distant est : %s\n", psd);

    /* -- Tant que (mr != "quitter") ET (me != "quitter") -- */
    while (strcmp(mr, "quitter") != 0 && strcmp(me, "quitter") != 0) {

        // Initialiser(mr, me)
        memset(mr, 0, BUF_SIZE);
        memset(me, 0, BUF_SIZE);

        // Afficher(psl)  — afficher notre pseudo comme invite
        printf("%s : ", psl);

        // Lire(me)  — saisir le message à envoyer
        fgets(me, BUF_SIZE, stdin);
        me[strcspn(me, "\n")] = '\0';

        // Send(D, me)
        send(D, me, strlen(me) + 1, 0);

        // Recv(D, mr)
        recv(D, mr, BUF_SIZE, 0);

        // Afficher(psd, mr)
        printf("%s : %s\n", psd, mr);
    }
    /* Fin Tant que */

    // Close(D)
    close(D);
    printf("Fin\n");
}

/* ============================================================
   Algorithme Programme Serveur Chatd  —  Image 1 (tableau)
   Entrées : @local (IP, port), messages à envoyer
   Sorties  : message reçu, @client (IP, port), message d'erreur
   ============================================================ */
int main(int argc, char *argv[]) {
    int Desc, Descc;
    struct sockaddr_in addr_l, addr_distant;
    socklen_t len = sizeof(addr_distant);
    int err;

    /* -- Début -- */

    // Initialisation des variables
    memset(&addr_l,       0, sizeof(addr_l));
    memset(&addr_distant, 0, sizeof(addr_distant));

    // Création du point de communication
    // Desc ← Socket(INET, Connecté, TCP)
    Desc = socket(AF_INET, SOCK_STREAM, 0);
    if (Desc < 0) {
        perror("Erreur socket");
        exit(1);
    }

    // Préparation de @local
    // Récupération num Port
    // Si (argc != 2) alors afficher("Usage: Chatd num_port") ; arreter()
    if (argc != 2) {
        fprintf(stderr, "Usage: %s num_port\n", argv[0]);
        exit(1);
    }

    // addr_l.port ← atoi(argv[1])
    addr_l.sin_family      = AF_INET;
    addr_l.sin_addr.s_addr = INADDR_ANY;
    addr_l.sin_port        = htons(atoi(argv[1]));

    // Configuration de l'adresse locale
    // err ← bind(desc, addr_l)
    err = bind(Desc, (struct sockaddr *)&addr_l, sizeof(addr_l));
    if (err != 0) {
        perror("Erreur bind");
        exit(1);
    }

    // err ← listen(desc, nbr_max)
    err = listen(Desc, NBR_MAX);
    if (err != 0) {
        perror("Erreur listen");
        exit(1);
    }

    printf("Serveur en attente sur le port %s...\n", argv[1]);

    /* -- Tant que vrai -- */
    while (1) {

        // Descc ← accept(desc, addr_distant)
        Descc = accept(Desc, (struct sockaddr *)&addr_distant, &len);

        // Si (Desc > 0) alors
        if (Descc > 0) {

            // Afficher("client connecté", addr_distant.IP, addr_distant.port)
            printf("Client connecté — IP : %s  Port : %d\n",
                   inet_ntoa(addr_distant.sin_addr),
                   ntohs(addr_distant.sin_port));

            // Chats(Descc)
            Chats(Descc);
        }
        // Fin si
    }
    /* Fin Tant que */

    close(Desc);
    return 0;
}
