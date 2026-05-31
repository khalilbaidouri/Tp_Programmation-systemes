#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage : %s N prog1 [prog2 ...]\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);

    if (N <= 0) {
        printf("N doit etre > 0\n");
        return 1;
    }

    int total = 0;

    // Créer N processus pour chaque programme
    for (int p = 2; p < argc; p++) {

        for (int i = 0; i < N; i++) {

            pid_t pid = fork();

            if (pid < 0) {
                perror("fork");

            } else if (pid == 0) {
                // Fils
                printf("Fils %d : lancement de %s\n",
                       getpid(), argv[p]);

                execlp(argv[p], argv[p], NULL);

                perror("execlp");
                exit(1);
            }

            total++;
        }
    }

    // Attendre tous les fils
    for (int i = 0; i < total; i++) {
        wait(NULL);
    }

    printf("Tous les processus sont termines.\n");

    return 0;
}
