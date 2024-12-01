#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>

void mostrar_contenido(int fd) {
    char buffer;
    ssize_t bytes_read;

    // Leer byte a byte desde la posición actual del marcador
    while ((bytes_read = read(fd, &buffer, 1)) > 0) {
        write(STDOUT_FILENO, &buffer, 1); // Escribir en la salida estándar
    }

    if (bytes_read < 0) {
        perror("Error leyendo el archivo");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    int opt;
    int n = 0;             // Número de bytes a saltar o leer
    int flag_e = 0;        // Bandera para la opción -e
    char *filename = NULL; // Nombre del archivo

    // Parsear los argumentos usando getopt
    while ((opt = getopt(argc, argv, "n:e")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg); // Convertir el valor de -n a entero
                break;
            case 'e':
                flag_e = 1; // Activar la bandera para leer los últimos N bytes
                break;
            default:
                fprintf(stderr, "Uso: %s [-n N] [-e] <archivo>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Obtener el nombre del archivo (el primer argumento no procesado por getopt)
    if (optind < argc) {
        filename = argv[optind];
    } else {
        fprintf(stderr, "Error: Se debe especificar un archivo.\n");
        exit(EXIT_FAILURE);
    }

    // Abrir el archivo
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error abriendo el archivo");
        exit(EXIT_FAILURE);
    }

    // Posicionar el marcador de posición con lseek
    if (flag_e) {
        // Leer los últimos N bytes
        if (lseek(fd, -n, SEEK_END) < 0) {
            perror("Error posicionando el marcador");
            close(fd);
            exit(EXIT_FAILURE);
        }
    } else {
        // Saltar los primeros N bytes
        if (lseek(fd, n, SEEK_SET) < 0) {
            perror("Error posicionando el marcador");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    // Mostrar el contenido del archivo desde la posición actual del marcador
    mostrar_contenido(fd);

    // Cerrar el archivo
    close(fd);

    return 0;
}
