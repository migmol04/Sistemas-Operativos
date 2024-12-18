#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define NUM_THREADS 10

struct thread_info {
    int thread_id;
};

void* thread_function(void* arg) {
    struct thread_info* tinfo = (struct thread_info*)arg;
    int thread_id = tinfo->thread_id;

    int fd_child = open("output.txt", O_RDWR);
    if (fd_child == -1) {
        perror("open (child)");
        pthread_exit(NULL);
    }

    off_t pos = thread_id * 5;  
    if (lseek(fd_child, pos, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        close(fd_child);
        pthread_exit(NULL);
    }

    char buffer[6];
    snprintf(buffer, 6, "%d%d%d%d%d", thread_id, thread_id, thread_id, thread_id, thread_id);
    if (write(fd_child, buffer, 5) != 5) {
        perror("write");
    }

    close(fd_child);
    pthread_exit(NULL);
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    struct thread_info tinfo[NUM_THREADS];
    int fd, i;

    // Creamos/truncamos el fichero y escribimos los primeros 5 ceros desde el hilo principal
    fd = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (write(fd, "00000", 5) != 5) {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);

    // Creamos los hilos del 1 al 9 (el 0 ya lo hemos escrito desde el main)
    for (i = 1; i < NUM_THREADS; i++) {
        tinfo[i].thread_id = i;
        if (pthread_create(&threads[i], NULL, thread_function, &tinfo[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // El hilo 0 no hace nada aquí porque su parte ya se escribió desde el main

    for (i = 1; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Ahora leemos el contenido del fichero
    fd = open("output.txt", O_RDONLY);
    if (fd == -1) {
        perror("open final read");
        exit(EXIT_FAILURE);
    }

    printf("File contents are:\n");
    char c;
    while (read(fd, &c, 1) > 0) {
        putchar(c);
    }
    printf("\n");

    close(fd);
    return 0;
}
