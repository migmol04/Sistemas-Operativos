#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

// Identificador global del proceso hijo
pid_t child_pid;

// Manejador para la señal SIGALRM
void handle_alarm(int sig) {
    if (child_pid > 0) {
        printf("Tiempo agotado. Enviando SIGKILL al proceso hijo (PID: %d)\n", child_pid);
        kill(child_pid, SIGKILL);  // Enviar SIGKILL al hijo
    }
}

// Configuración para ignorar SIGINT
void ignore_sigint(int sig) {
    printf("SIGINT ignorado por el proceso padre.\n");
}

int main(int argc, char **argv) {
    // Validar los argumentos
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <comando> [argumentos...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Crear el proceso hijo
    child_pid = fork();  //fork() divide el programa en dos: Padre: Recibe el PID del hijo. Hijo: Recibe 0 como valor de retorno.

    if (child_pid == -1) { //hubo un error
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        // Código del proceso hijo
        printf("Hijo ejecutando: %s\n", argv[1]);
        execvp(argv[1], &argv[1]);  // Reemplazar el ejecutable execvp reemplaza el código del proceso hijo con el programa especificado en argv[1] y sus argumentos. El segundo argumento (&argv[1]) es un array de cadenas que contiene arg1 arg2..
        perror("execvp");  // Solo se ejecuta si execvp falla
        exit(EXIT_FAILURE);
    } else {
        // Código del proceso padre

        // Configurar el manejador de SIGALRM
        struct sigaction sa; //Configura el manejador de SIGALRM para que ejecute la función handle_alarm. sa_handler: Un puntero a la función que se ejecutará cuando se reciba la señal.   sa_mask: Especifica un conjunto de señales que se bloquearán mientras el manejador esté ejecutándose  sa_flags: Permite configurar opciones adicionales para el manejo de la señal
        sa.sa_handler = handle_alarm;
        sigemptyset(&sa.sa_mask); //sigemptyset inicializa el conjunto de señales bloqueadas a vacío. no se bloqueará ninguna señal adicional mientras se ejecuta el manejador handle_alarm.
        sa.sa_flags = 0;
        if (sigaction(SIGALRM, &sa, NULL) == -1) {  //Asocia la estructura configurada (sa) con la señal SIGALRM. A partir de este momento, cuando el programa reciba la señal SIGALRM, se ejecutará la función handle_alarm
            perror("sigaction");
            exit(EXIT_FAILURE);
        }

        // Configurar ignorar SIGINT
         if (signal(SIGINT, ignore_sigint) == SIG_ERR) {
            perror("signal SIGINT");
            exit(EXIT_FAILURE);
        } //Si el programa recibe SIGINT, ejecutará la función ignore_sigint, que simplemente imprime un mensaje

        // Programar alarma para 5 segundos
        alarm(5);

       int status;
pid_t terminated_pid;

do {
    terminated_pid = waitpid(child_pid, &status, 0);
} while (terminated_pid == -1 && errno == EINTR);

if (terminated_pid == -1) {
    perror("waitpid");
    exit(EXIT_FAILURE);
}


        // Comprobar cómo terminó el proceso hijo
        if (WIFEXITED(status)) {
            printf("Hijo (PID: %d) terminó normalmente con código de salida %d.\n", child_pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Hijo (PID: %d) fue terminado por la señal %d.\n", child_pid, WTERMSIG(status));
        } else {
            printf("Hijo (PID: %d) terminó de manera desconocida.\n", child_pid);
        }

        return 0;
    }
}
