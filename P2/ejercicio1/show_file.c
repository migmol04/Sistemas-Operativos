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
	while ((c = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0) {
		/* Print byte to stdout */
		 bytesWritten = fwrite(buffer, sizeof(char), c, stdout);

		if (bytesWritten==0){
			fclose(file);
			err(3,"putc() failed!!");
		}
	}


	fclose(file);
	return 0;
}
