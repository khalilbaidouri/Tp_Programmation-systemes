#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int p1[2], p2[2];
    int valeurs[5] = {1, 2, 3, 4, 5};
    int val, double_val;

    if (pipe(p1) == -1 || pipe(p2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        /* ===== FILS ===== */
        close(p1[1]); /* ferme écriture p1 */
        close(p2[0]); /* ferme lecture p2  */

        for (int i = 0; i < 5; i++) {
            read(p1[0], &val, sizeof(int));
            printf("Fils reçoit : %d\n", val);
            double_val = val * 2;
            write(p2[1], &double_val, sizeof(int));
        }

        close(p1[0]);
        close(p2[1]);
        exit(EXIT_SUCCESS);

    } else {
        /* ===== PERE ===== */
        close(p1[0]); /* ferme lecture p1  */
        close(p2[1]); /* ferme écriture p2 */

        for (int i = 0; i < 5; i++) {
            write(p1[1], &valeurs[i], sizeof(int));
            read(p2[0], &double_val, sizeof(int));
            printf("Pere reçoit double : %d\n", double_val);
        }

        close(p1[1]);
        close(p2[0]);
        wait(NULL);
    }

    return 0;
}
