#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BLOCK_SIZE 512 // Tamaño de bloque en bytes

void copy(int fdo, int fdd)
{
    char buffer[BLOCK_SIZE]; // Buffer para almacenamiento intermedio
    ssize_t bytes_read, bytes_written;

	  // Leer del archivo origen y escribir en el archivo destino
    while ((bytes_read = read(fdo, buffer, BLOCK_SIZE)) > 0) {
        // Escribir tantos bytes como se leyeron
        bytes_written = write(fdd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Error escribiendo en el archivo destino");
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read < 0) { // Comprobar errores en la lectura
        perror("Error leyendo del archivo origen");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
	int fdo, fdd;
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <archivo_origen> <archivo_destino>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

	 // Abrir el archivo origen en modo lectura
    fdo = open(argv[1], O_RDONLY);
    if (fdo < 0) {
        perror("Error abriendo el archivo origen");
        exit(EXIT_FAILURE);
    }

	// Abrir el archivo destino en modo escritura (crear si no existe, truncar si existe)
    fdd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644); /*0644 se descompone así:
0: Indica que es un valor octal.
6: Permisos del propietario (lectura y escritura: 4 + 2).
4: Permisos del grupo (solo lectura: 4).
4: Permisos de otros (solo lectura: 4).*/
    if (fdd < 0) {
        perror("Error abriendo el archivo destino");
        close(fdo); // Asegurarse de cerrar el archivo origen si hay error
        exit(EXIT_FAILURE);
    }

	// Llamar a la función de copia
    copy(fdo, fdd);

    // Cerrar los archivos
    close(fdo);
    close(fdd);

    printf("Copia completada: %s -> %s\n", argv[1], argv[2]);
    return 0;

	return 0;
}
