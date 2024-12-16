#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor
 *
 * The loadstr() function must allocate memory from the heap to store
 * the contents of the string read from the FILE.
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()).
 *
 * Returns: !=NULL if success, NULL if error
 */
char *loadstr(FILE *file) {
    char *c = malloc(sizeof(char));
    int bytesRead, stringBytes = 0;

    if (!c) {
        perror("Error al reservar memoria para el carácter");
        return NULL;
    }

    // Leer carácter por carácter hasta encontrar '\0'
    do {
        bytesRead = fread(c, 1, 1, file);
        if (bytesRead == 0) { // Verificar si se alcanzó el EOF
            free(c);
            return NULL;
        }
        stringBytes++;
    } while (*c != '\0');

    // Reservar memoria para toda la cadena
    char *str = malloc(sizeof(char) * stringBytes);
    if (!str) {
        perror("Error al reservar memoria para la cadena");
        free(c);
        return NULL;
    }

    // Retroceder en el archivo para leer toda la cadena
    fseek(file, -stringBytes, SEEK_CUR); /*La función fseek se utiliza para mover el puntero de posición dentro de un archivo. 
    Este puntero indica desde qué posición se leerán o escribirán datos la próxima vez.
    FILE *stream

El puntero al archivo que se está manejando, por ejemplo, file.
long offset

Número de bytes a mover desde la posición indicada por origin.
Puede ser positivo (mover hacia adelante) o negativo (mover hacia atrás).
int origin

Indica desde dónde se va a mover el puntero. Las opciones más comunes son:
SEEK_SET: Mover el puntero desde el inicio del archivo.
SEEK_CUR: Mover el puntero desde la posición actual del puntero.
SEEK_END: Mover el puntero desde el final del archivo.*/
    fread(str, 1, stringBytes, file);

    // Liberar memoria del carácter temporal
    free(c);
    return str;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error al abrir el archivo");
        return 1;
    }

    char *str;
    str = loadstr(file);
    while (str != NULL) {
        printf("%s\n", str);
        free(str); 
        str = loadstr(file);
    }

    fclose(file);
    return 0;
}
