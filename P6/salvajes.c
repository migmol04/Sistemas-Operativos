#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#define NUMITER 3             // Número de veces que cada salvaje comerá
#define SHM_NAME "/caldero"   // Nombre de la memoria compartida
#define SEM_NAME "/semaforo"  // Nombre del semáforo

sem_t *sem;                   // Semáforo para sincronización
int *caldero;                 // Puntero a la memoria compartida que representa el caldero

// Función para obtener una ración del caldero
int getServingsFromPot(void)
{
    sem_wait(sem);  // Espera a acceder al caldero

    if (*caldero > 0)
    {
        (*caldero)--;  // Toma una ración del caldero
        printf("Salvaje %lu toma una ración. Quedan %d raciones\n", (unsigned long)getpid(), *caldero);
        sem_post(sem);  // Libera el semáforo para otros salvajes
        return 1;       // Indica que se ha tomado una ración exitosamente
    }
    else
    {
        printf("Salvaje %lu encuentra el caldero vacío. Invoca al cocinero.\n", (unsigned long)getpid());
        sem_post(sem);  // Libera el semáforo para que el cocinero lo rellene
        return 0;       // Indica que el caldero estaba vacío
    }
}

// Función para simular el acto de comer
void eat(void)
{
    printf("Salvaje %lu está comiendo\n", (unsigned long)getpid());
    sleep(rand() % 3 + 1);  // Simula el tiempo que tarda en comer (1-3 segundos)
}

// Función principal de los salvajes
void savages(void)
{
    for (int i = 0; i < NUMITER; i++)
    {
        while (!getServingsFromPot())  // Intenta tomar una ración hasta que lo logre
        {
            sleep(1);  // Espera un momento antes de intentar de nuevo
        }
        eat();
    }
}

int main(int argc, char *argv[])
{
    srand(getpid());  // Semilla para generar números aleatorios

    // Abrir la memoria compartida creada por el cocinero
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("Error abriendo memoria compartida");
        exit(EXIT_FAILURE);
    }

    // Mapear la memoria compartida en el espacio del proceso
    caldero = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (caldero == MAP_FAILED)
    {
        perror("Error mapeando memoria compartida");
        exit(EXIT_FAILURE);
    }

    // Abrir el semáforo creado por el cocinero
    sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED)
    {
        perror("Error abriendo semáforo");
        exit(EXIT_FAILURE);
    }

    // Ejecutar la función de los salvajes
    savages();

    // Cerrar los recursos
    sem_close(sem);
    munmap(caldero, sizeof(int));

    printf("Salvaje %lu ha terminado de comer.\n", (unsigned long)getpid());

    return 0;
}
