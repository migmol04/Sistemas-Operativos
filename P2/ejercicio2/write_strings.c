#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <file_name> <string1> [string2 ... stringN]\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "wb");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    for (int i = 2; i < argc; i++) {
        char *word = argv[i];
        do {
            fwrite(word, sizeof(char), 1, file);
        } while (*(word++) != '\0');
    }

    fclose(file);
    return 0;
}
