#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

const char *data = "Donnees sauvegardees avant SIGINT\n";

void handler_sigint(int sig) {
    int fd = open("sauvegarde.txt",
                  O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        _exit(EXIT_FAILURE);
    }
    write(fd, data, strlen(data));
    close(fd);
    printf("\nDonnees sauvegardees dans sauvegarde.txt\n");
    _exit(EXIT_SUCCESS);
}

int main(void) {
    signal(SIGINT, handler_sigint);
    printf("Programme en cours... Appuyez sur Ctrl-C\n");
    while (1) {
        pause(); /* attend un signal */
    }
    return 0;
}
