#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_COMMANDS 100

pid_t launch_command(char **argv) {
    pid_t pid = fork();

    if (pid == -1) {  // Error al crear el proceso hijo
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {  // Código del proceso hijo
        if (execvp(argv[0], argv) == -1) {  // Ejecuta el comando  
        /*int execvp(const char *file, char *const argv[]); file: Nombre del comando a ejecutar (ej. "ls").
argv: Un arreglo de punteros a cadenas que representan los argumentos del comando. Debe terminar con NULL.*/ 
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    // Código del padre: devuelve el PID del hijo
    return pid;
}



char **parse_command(const char *cmd, int* argc) {
    // Allocate space for the argv array (initially with space for 10 args)
    size_t argv_size = 10;
    const char *end;
    size_t arg_len; 
    int arg_count = 0;
    const char *start = cmd;
    char **argv = malloc(argv_size * sizeof(char *));

    if (argv == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while (*start && isspace(*start)) start++; // Skip leading spaces

    while (*start) {
        // Reallocate more space if needed
        if (arg_count >= argv_size - 1) {  // Reserve space for the NULL at the end
            argv_size *= 2;
            argv = realloc(argv, argv_size * sizeof(char *));
            if (argv == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        // Find the start of the next argument
        end = start;
        while (*end && !isspace(*end)) end++;

        // Allocate space and copy the argument
        arg_len = end - start;
        argv[arg_count] = malloc(arg_len + 1);

        if (argv[arg_count] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strncpy(argv[arg_count], start, arg_len);
        argv[arg_count][arg_len] = '\0';  // Null-terminate the argument
        arg_count++;

        // Move to the next argument, skipping spaces
        start = end;
        while (*start && isspace(*start)) start++;
    }

    argv[arg_count] = NULL; // Null-terminate the array

    (*argc)=arg_count; // Return argc

    return argv;
}

int main(int argc, char *argv[]) {
    int opt;
    int background_mode = 0;
    char *script_file = NULL;
    char *command = NULL;
    pid_t pids[MAX_COMMANDS];  // Para la opción -b
    int num_commands = 0;

    while ((opt = getopt(argc, argv, "x:s:b")) != -1) {
        switch (opt) {
            case 'x':  // Ejecutar un comando
                command = optarg; //capturamos el comando
                int cmd_argc;
                char **cmd_argv = parse_command(command, &cmd_argc); //parse_command: Divide el comando en argumentos (por ejemplo, "ls -l" se convierte en {"ls", "-l", NULL}).
                pid_t pid = launch_command(cmd_argv); //Crea un proceso hijo para ejecutar el comando con execvp
                int status;
                waitpid(pid, &status, 0);//Espera a que el proceso hijo termine y obtiene su estado
                /*pid_t waitpid(pid_t pid, int *status, int options);
                Descripción: Hace que el proceso padre espere a que un proceso hijo termine.

Parámetros:

pid: El PID del hijo a esperar (-1 para esperar a cualquier hijo).
status: Puntero donde se almacenará el estado del proceso hijo.
options: Opciones adicionales (ej. 0 para comportamiento por defecto).
Devuelve:

El PID del hijo que terminó.
-1 si hay un error.*/
                printf("@@ Command terminated (pid: %d, status: %d)\n", pid, WEXITSTATUS(status));

                // Liberar memoria
                for (int i = 0; i < cmd_argc; i++) {
                    free(cmd_argv[i]);
                }
                free(cmd_argv);
                break;

            case 's':  // Leer comandos de un fichero
                script_file = optarg; //script_file = optarg: Captura el nombre del archivo.
                FILE *file = fopen(script_file, "r"); //script_file = optarg: Captura el nombre del archivo.
                if (!file) {
                    perror("fopen");
                    exit(EXIT_FAILURE);
                }

                char line[1024];
                int command_index = 0;
                while (fgets(line, sizeof(line), file)) {  //fgets: Lee cada línea del archivo.
                    line[strcspn(line, "\n")] = '\0';  // Quitar el salto de línea
                    int cmd_argc;
                    char **cmd_argv = parse_command(line, &cmd_argc); //parse_command: Convierte la línea en un array de argumentos.

                    if (background_mode) {
                        if (num_commands < MAX_COMMANDS) {
                            pids[num_commands++] = launch_command(cmd_argv); //Lanza cada comando en segundo plano, almacenando su PID en el array pids
                            printf("@@ Running command #%d: %s\n", command_index++, line);
                        } else {
                            fprintf(stderr, "Error: Too many commands for background mode.\n");
                            break;
                        }
                    } else {  //Ejecuta el comando con launch_command y espera a que termine usando waitpid
                        printf("@@ Running command #%d: %s\n", command_index++, line);
                        pid_t pid = launch_command(cmd_argv);
                        int status;
                        waitpid(pid, &status, 0);
                        printf("@@ Command #%d terminated (pid: %d, status: %d)\n", command_index - 1, pid, WEXITSTATUS(status));
                    }

                    // Liberar memoria
                    for (int i = 0; i < cmd_argc; i++) {
                        free(cmd_argv[i]);
                    }
                    free(cmd_argv);
                }

                fclose(file);

                if (background_mode) {   //Espera a que todos los procesos lanzados en background terminen y muestra su estado.
                    for (int i = 0; i < num_commands; i++) {
                        int status;
                        waitpid(pids[i], &status, 0);
                        printf("@@ Command #%d terminated (pid: %d, status: %d)\n", i, pids[i], WEXITSTATUS(status));
                    }
                }
                break;

            case 'b':  // Activar modo background
                background_mode = 1;
                break;

            default:
                fprintf(stderr, "Usage: %s -x <command> | -s <file> [-b]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}