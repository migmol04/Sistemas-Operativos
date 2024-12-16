#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#define BUFFER_SIZE 40

int main(int argc, char* argv[]) {
	FILE* file=NULL;
	int c,ret, bytesWritten;
	char buffer[BUFFER_SIZE];

	if (argc!=2) {
		fprintf(stderr,"Usage: %s <file_name>\n",argv[0]);
		exit(1);
	}

	/* Open file */
	if ((file = fopen(argv[1], "r")) == NULL)
		err(2,"The input file %s could not be opened",argv[1]);

/*	 Read file byte by byte
	while ((c = getc(file)) != EOF) {
		Print byte to stdout 
		ret=putc((unsigned char) c, stdout);

		if (ret==EOF){
			fclose(file);
			err(3,"putc() failed!!");
		}
	}
*/
	while ((c = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0) {  /*size_t fread(void *ptr, size_t size, size_t count, FILE *stream); Parámetros
void *ptr:
Un puntero al bloque de memoria donde se almacenarán los datos leídos.

size_t size:
El tamaño (en bytes) de cada elemento a leer.

size_t count:
La cantidad de elementos de tamaño size que se leerán.

FILE *stream:
El puntero al archivo del cual se van a leer los datos.*/ 
		/* Print byte to stdout */
		 bytesWritten = fwrite(buffer, sizeof(char), c, stdout);  /*size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);
const void *ptr:
Un puntero al bloque de memoria que contiene los datos a escribir.

size_t size:
El tamaño (en bytes) de cada elemento a escribir.

size_t count:
La cantidad de elementos de tamaño size que se escribirán.

FILE *stream:
El puntero al archivo donde se escribirán los datos.

*/
		if (bytesWritten==0){
			fclose(file);
			err(3,"putc() failed!!");
		}
	}


	fclose(file);
	return 0;
}
