#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#define CAPACITY 5
#define VIPSTR(vip) ((vip) ? "  vip  " : "not vip")

int inside = 0;          // Número de clientes dentro de la discoteca
int waiting_vip = 0;     // Número de clientes VIP esperando
int waiting_normal = 0;  // Número de clientes normales esperando
int waiting_special = 0; // Número de clientes special esperando
int special_inside = 0;  // Indica si dentro hay al menos un cliente special

pthread_mutex_t mutex;   //Asegura que sólo un hilo pueda acceder a la sección crítica (modificación de variables compartidas) a la vez
pthread_cond_t cond_vip; // Variable de condición para que los clientes VIP esperen hasta que puedan entrar.
pthread_cond_t cond_normal; //Variable de condición para que los clientes normales esperen hasta que puedan entrar.
pthread_cond_t cond_special;

static inline const char *client_type_str(int type) {
    switch(type) {
        case 0: return "normal ";
        case 1: return "  vip  ";
        case 2: return "special";
        default: return "??????";
    }
}

void enter_normal_client(int id)
{
    pthread_mutex_lock(&mutex); //Bloquea el mutex para asegurar acceso exclusivo a las variables compartidas
    waiting_normal++;  // Incrementa el número de normales esperando

      // Mientras la discoteca esté llena, haya VIPs esperando, o haya especiales esperando o dentro, esperar
    while (inside >= CAPACITY || waiting_vip > 0 || waiting_special > 0 || special_inside == 1) {
        printf("Client %2d (normal ) waiting to enter\n", id);
        pthread_cond_wait(&cond_normal, &mutex);
    }

    waiting_normal--;  // Decrementa el número de normales esperando
    inside++;          // Incrementa el número de clientes dentro
    printf("Client %2d (not vip) entered the disco (inside: %d)\n", id, inside);

    pthread_mutex_unlock(&mutex); //desbloquea
}

void enter_vip_client(int id)
{
    pthread_mutex_lock(&mutex); //Bloquea el mutex para asegurar acceso exclusivo a las variables compartida
    waiting_vip++;  // Incrementa el número de VIPs esperando

     // Mientras la discoteca esté llena, haya especiales esperando o dentro, esperar
    while (inside >= CAPACITY || waiting_special > 0 || special_inside == 1) {
        printf("Client %2d (  vip  ) waiting to enter\n", id);
        pthread_cond_wait(&cond_vip, &mutex);
    }

    waiting_vip--;  // Decrementa el número de VIPs esperando
    inside++;       // Incrementa el número de clientes dentro
    printf("Client %2d (  vip  ) entered the disco (inside: %d)\n", id, inside);

    pthread_mutex_unlock(&mutex);  //desbloquea
}

void enter_special_client(int id)
{
    pthread_mutex_lock(&mutex);
    waiting_special++;

    // Los especiales solo pueden entrar si la discoteca está totalmente vacía
    while (inside > 0) {
        printf("Client %2d (special) waiting to enter (disco not empty)\n", id);
        pthread_cond_wait(&cond_special, &mutex);
    }

    waiting_special--;
    inside++;
    special_inside = 1; // Marca que hay especiales dentro
    printf("Client %2d (special) entered the disco (inside: %d)\n", id, inside);

    pthread_mutex_unlock(&mutex);
}



void dance(int id, int isvip)
{
	printf("Client %2d (%s) dancing in disco\n", id, VIPSTR(isvip));
	sleep((rand() % 3) + 1);
}

void disco_exit(int id, int type)
{
    pthread_mutex_lock(&mutex);

    inside--;
    printf("Client %2d (%s) exited the disco (inside: %d)\n", id, client_type_str(type), inside);

    // Al salir un cliente, si la disco queda vacía y había special esperando, ellos tienen prioridad absoluta.
    if (inside == 0) {
        if (special_inside == 1) {
            // Si se fueron todos los special, restaurar el flag
            if (inside == 0)
                special_inside = 0;
        }

        // Prioridad: special > vip > normal
        if (waiting_special > 0) {
            // Despertar a todos los special, ya que pueden entrar varios si hay capacidad
            pthread_cond_broadcast(&cond_special);
        } else if (waiting_vip > 0 && special_inside == 0) {
            pthread_cond_signal(&cond_vip);
        } else if (waiting_normal > 0 && special_inside == 0) {
            pthread_cond_signal(&cond_normal);
        }
    } else {
        // Si la discoteca no está vacía:
        // Si hay special esperando, no puede entrar nadie hasta que la disco se vacíe, así que no señalizamos special ahora.
        // En cambio, si no hay special esperando ni dentro:
        if (special_inside == 0 && waiting_special == 0) {
            // Dar prioridad a vip
            if (waiting_vip > 0) {
                pthread_cond_signal(&cond_vip);
            } else if (waiting_normal > 0) {
                pthread_cond_signal(&cond_normal);
            }
        }
    }

    pthread_mutex_unlock(&mutex);
}
void *client(void *arg)
{
    int id = ((int *)arg)[0];
    int type = ((int *)arg)[1]; // 0: normal, 1: vip, 2: special

    if (type == 2)
        enter_special_client(id);
    else if (type == 1)
        enter_vip_client(id);
    else
        enter_normal_client(id);

    dance(id, type);
    disco_exit(id, type);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <fichero_entrada>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r"); //Abre el fichero en modo lectura.
    if (!file) {
        perror("Error al abrir el fichero");
        exit(EXIT_FAILURE);
    }

    int M;
    fscanf(file, "%d", &M); //Lee el número total de clientes a crear (M).

    pthread_t threads[M];
    int args[M][2];  // args[i][0] = id, args[i][1] = tipo (0 normal, 1 vip, 2 special)

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_vip, NULL);
    pthread_cond_init(&cond_normal, NULL);
    pthread_cond_init(&cond_special, NULL);

    for (int i = 0; i < M; i++)
    {
        int client_type;
        fscanf(file, "%d", &client_type); // 0, 1, o 2
        args[i][0] = i + 1;       
        args[i][1] = client_type; // Guardamos directamente el tipo, sin convertir a booleano

        pthread_create(&threads[i], NULL, client, (void *)args[i]);
    }

    fclose(file);

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < M; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Destruir mutex y variables de condición
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_vip);
    pthread_cond_destroy(&cond_normal);
    pthread_cond_destroy(&cond_special);

    return 0;
}