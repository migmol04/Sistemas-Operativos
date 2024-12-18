#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "setargs.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <fichero>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        int n = setargs(buffer, NULL); // Contamos cuántas palabras hay

        if (n == 0) {
            // Línea vacía o solo espacios, no ejecutamos nada
            continue;
        }

        char **cargv = malloc((n+1) * sizeof(char *));
        if (!cargv) {
            perror("malloc");
            continue;
        }

        cargv[n] = NULL; // Aseguramos el NULL final

        // Ahora sí, tokenizamos la cadena
        setargs(buffer, cargv);

        // Crear un proceso hijo para ejecutar cargv[0] con argumentos
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            free(cargv);
            continue;
        }

        if (pid == 0) {
            // Proceso hijo: ejecutar el comando
            execvp(cargv[0], cargv);
            perror("execvp");
            exit(1);
        } else {
            // Proceso padre: esperar al hijo
            wait(NULL);
        }

        free(cargv);
    }

    fclose(fp);
    return 0;
}
