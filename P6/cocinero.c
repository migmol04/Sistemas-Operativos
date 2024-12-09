#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#define M 10                 // Número de raciones por ciclo
#define SHM_NAME "/caldero"  // Nombre de la memoria compartida
#define SEM_NAME "/semaforo" // Nombre del semáforo

int finish = 0;              // Variable para terminar el bucle del cocinero
int *caldero;                // Puntero a la memoria compartida
sem_t *sem;                  // Semáforo

// Función para rellenar el caldero
void putServingsInPot(int servings)
{
    *caldero = servings; // Se llena el caldero con 'servings' raciones
    printf("Cocinero: Se han puesto %d raciones en el caldero\n", servings);
    sem_post(sem); // Libera el semáforo para que los salvajes puedan acceder al caldero
}

// Bucle principal del cocinero
void cook(void)
{
    while (!finish)
    {
        sem_wait(sem);        // Espera hasta que los salvajes bloqueen el semáforo (indica caldero vacío)
        putServingsInPot(M);  // Rellena el caldero con M raciones
    }
}

// Manejador de señales para terminar el programa de forma segura
void handler(int signo)
{
    finish = 1;  // Señal para terminar el bucle principal
}

int main(int argc, char *argv[])
{
    // Configurar el manejador de señales para SIGINT (Ctrl+C) y SIGTERM
    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    // Crear memoria compartida
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("Error creando memoria compartida");
        exit(EXIT_FAILURE);
    }
    ftruncate(shm_fd, sizeof(int)); // Ajusta el tamaño de la memoria compartida
    caldero = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (caldero == MAP_FAILED)
    {
        perror("Error mapeando memoria compartida");
        exit(EXIT_FAILURE);
    }

    // Inicializar el semáforo
    sem = sem_open(SEM_NAME, O_CREAT, 0666, 0); // Inicializado en 0 para que el cocinero espere
    if (sem == SEM_FAILED)
    {
        perror("Error creando semáforo");
        exit(EXIT_FAILURE);
    }

    printf("Cocinero listo. Esperando a que los salvajes necesiten más comida...\n");

    // Ejecutar el bucle del cocinero
    cook();

    // Limpiar recursos al finalizar
    sem_close(sem);
    sem_unlink(SEM_NAME);
    munmap(caldero, sizeof(int));
    shm_unlink(SHM_NAME);

    printf("Cocinero finaliza y limpia recursos.\n");
    return 0;
}
