#include <stdio.h>
#include <ctype.h>   // Para isspace
#include "setargs.h"

int setargs(char *args, char **argv) {
    // Esta función debe:
    // 1. Ignorar espacios iniciales.
    // 2. Contar el número de subcadenas separadas por espacios.
    // 3. Si argv != NULL, reemplazar espacios por '\0' y asignar punteros en argv.
    
    int count = 0;
    char *p = args;
    
    // 1. Ignorar espacios iniciales
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }

    if (argv == NULL) {
        // Sólo contamos las subcadenas, no modificamos la cadena.
        while (*p) {
            // Hemos encontrado el inicio de una palabra
            if (!isspace((unsigned char)*p)) {
                count++;
                // Avanzar hasta el final de la palabra
                while (*p && !isspace((unsigned char)*p)) {
                    p++;
                }
                // Saltar espacios tras la palabra
                while (*p && isspace((unsigned char)*p)) {
                    p++;
                }
            }
        }
        return count;
    }
    else {
        // Aquí, además de contar, modificamos la cadena y apuntamos argv.
        // Recorremos de nuevo (desde el principio de args) ya que la cadena podría haber cambiado.
        // Sin embargo, si queremos reutilizar el código, simplemente reseteamos p al inicio,
        // ignoramos espacios de nuevo y hacemos el proceso tokenizando.
        
        p = args;
        // Ignorar espacios iniciales de nuevo (por seguridad)
        while (*p && isspace((unsigned char)*p)) {
            p++;
        }

        while (*p) {
            // Inicio de una palabra
            if (!isspace((unsigned char)*p)) {
                // Apuntamos argv[count] a este inicio de palabra
                argv[count] = p;
                count++;
                
                // Avanzar hasta el final de la palabra
                while (*p && !isspace((unsigned char)*p)) {
                    p++;
                }
                
                // Si no es el final de la cadena, *p es un espacio.
                // Lo convertimos en '\0' para terminar la palabra.
                if (*p && isspace((unsigned char)*p)) {
                    *p = '\0';
                    p++;
                }
                
                // Saltar espacios hasta la siguiente palabra (o fin)
                while (*p && isspace((unsigned char)*p)) {
                    p++;
                }
            }
        }
        return count;
    }

    
    return count;
}
