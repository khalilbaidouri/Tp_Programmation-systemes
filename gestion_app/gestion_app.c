#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct Noeud {
    char *nom;
    pid_t pid;
    struct Noeud *suivant;
} Noeud;

/* Ajouter un programme à la liste */
void ajouter(Noeud **tete, char *nom)
{
    Noeud *n = malloc(sizeof(Noeud));

    n->nom = strdup(nom);
    n->pid = 0;
    n->suivant = NULL;

    if (*tete == NULL) {
        *tete = n;
        return;
    }

    Noeud *tmp = *tete;

    while (tmp->suivant != NULL) {
        tmp = tmp->suivant;
    }

    tmp->suivant = n;
}

/* Afficher la liste */
void afficher(Noeud *tete)
{
    printf("\nListe des applications :\n");

    while (tete != NULL) {
        printf("%s (PID : %d)\n", tete->nom, tete->pid);
        tete = tete->suivant;
    }
}

/* Lancer les applications */
void lancer(Noeud *tete)
{
    while (tete != NULL) {

        pid_t pid = fork();

        if (pid == 0) {
            printf("Lancement de %s\n", tete->nom);

            execlp(tete->nom, tete->nom, NULL);

            perror("execlp");
            exit(1);
        }

        tete->pid = pid;
        tete = tete->suivant;
    }
}

/* Attendre tous les fils */
void attendre(Noeud *tete)
{
    while (tete != NULL) {

        if (tete->pid > 0) {
            waitpid(tete->pid, NULL, 0);
            printf("%s termine\n", tete->nom);
        }

        tete = tete->suivant;
    }
}

/* Libérer la mémoire */
void detruire(Noeud *tete)
{
    Noeud *tmp;

    while (tete != NULL) {
        tmp = tete;
        tete = tete->suivant;

        free(tmp->nom);
        free(tmp);
    }
}

int main(int argc, char *argv[])
{
    FILE *source;
    Noeud *liste = NULL;
    char ligne[100];

    /* Fichier ou clavier */
    if (argc > 1) {
        source = fopen(argv[1], "r");

        if (source == NULL) {
            perror("fichier");
            return 1;
        }
    } else {
        source = stdin;
        printf("Entrer les applications (Ctrl+D pour finir):\n");
    }

    /* Lire les applications */
    while (fgets(ligne, sizeof(ligne), source)) {

        ligne[strcspn(ligne, "\n")] = '\0';

        if (strlen(ligne) == 0)
            continue;

        ajouter(&liste, ligne);
    }

    if (source != stdin)
        fclose(source);

    if (liste == NULL) {
        printf("Aucune application\n");
        return 1;
    }

    afficher(liste);

    lancer(liste);

    afficher(liste);

    attendre(liste);

    detruire(liste);

    printf("Fin du programme\n");

    return 0;
}
