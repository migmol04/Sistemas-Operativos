#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_CMDS 1024

pid_t run_command(const char* command) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Proceso hijo
        execl("/bin/bash", "bash", "-c", command, (char*)NULL);
        perror("execl");
        exit(1);
    }
    // Proceso padre
    return pid;
}

int main(int argc, char *argv[]) {
    int opt;
    char *x_command = NULL;
    char *s_file = NULL;
    int b_flag = 0;

    // Procesar opciones
    // Aquí, notar que 'b' no requiere argumento, 'x' y 's' sí.
    while ((opt = getopt(argc, argv, "x:s:b")) != -1) {
        switch (opt) {
            case 'x':
                x_command = optarg;
                break;
            case 's':
                s_file = optarg;
                break;
            case 'b':
                b_flag = 1;
                break;
            default:
                fprintf(stderr, "Uso: %s [-x comando] [-s fichero] [-b]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Caso 1: Solo -x (sin -s ni -b)
    if (x_command && !s_file) {
        pid_t p = run_command(x_command);
        waitpid(p, NULL, 0);
        return EXIT_SUCCESS;
    }

    // Caso 2: -s sin -b (ejecución secuencial)
    if (s_file && !b_flag) {
        FILE *f = fopen(s_file, "r");
        if (!f) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char linea[1024];
        while (fgets(linea, sizeof(linea), f)) {
            linea[strcspn(linea, "\n")] = '\0';
            if (strlen(linea) == 0) continue; // saltar líneas vacías

            pid_t pid = run_command(linea);
            int status;
            waitpid(pid, &status, 0);
        }
        fclose(f);
        return EXIT_SUCCESS;
    }

    // Caso 3: -s y -b (ejecución en batch sin esperar entre comandos)
    // Se lanzan todos los comandos y luego se esperan todos a que terminen.
    if (s_file && b_flag) {
        FILE *f = fopen(s_file, "r");
        if (!f) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char linea[1024];
        pid_t pid_array[MAX_CMDS];
        int num_cmds = 0;

        // Leer todas las líneas y lanzar sin esperar
        while (fgets(linea, sizeof(linea), f)) {
            linea[strcspn(linea, "\n")] = '\0';
            if (strlen(linea) == 0) continue;

            pid_t p = run_command(linea);
            pid_array[num_cmds++] = p;
        }
        fclose(f);

        // Esperar a que terminen todos los comandos
        int completed = 0;
        while (completed < num_cmds) {
            int status;
            pid_t done = wait(&status);
            // Buscar done en pid_array
            int i;
            for (i = 0; i < num_cmds; i++) {
                if (pid_array[i] == done) {
                    break;
                }
            }
            int exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            printf("@@ Command #%d terminated (pid: %d, status: %d)\n",
                   i+1, (int)done, exit_status);
            completed++;
        }
        return EXIT_SUCCESS;
    }

    // Si llega aquí, no se ha especificado ni -x ni -s correctamente
    fprintf(stderr, "Uso: %s [-x comando] [-s fichero] [-b]\n", argv[0]);
    return EXIT_FAILURE;
}
