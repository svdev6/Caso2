#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <omp.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s tamaño_de_matrices verbose(0 o 1) num_procesos\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int verbose = atoi(argv[2]);
    int num_processes = atoi(argv[3]);

    int shmid_a, shmid_b, shmid_c;
    key_t key = IPC_PRIVATE;
    
    // Crear matrices en memoria compartida
    shmid_a = shmget(key, N * N * sizeof(int), IPC_CREAT | 0666);
    shmid_b = shmget(key, N * N * sizeof(int), IPC_CREAT | 0666);
    shmid_c = shmget(key, N * N * sizeof(int), IPC_CREAT | 0666);

    int *matriz_a = (int *)shmat(shmid_a, NULL, 0);
    int *matriz_b = (int *)shmat(shmid_b, NULL, 0);
    int *matriz_c = (int *)shmat(shmid_c, NULL, 0);

    srand(time(NULL));

    // Inicializar las matrices
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matriz_a[i * N + j] = rand() % 10 + 1;
            matriz_b[i * N + j] = rand() % 10 + 1;
            matriz_c[i * N + j] = 0;
        }
    }

    double start_time = omp_get_wtime();

    int rows_per_process = N / num_processes;
    int remaining_rows = N % num_processes;

    #pragma omp parallel num_threads(num_processes)
    {
        #pragma omp for
        for (int p = 0; p < num_processes; p++) {
            pid_t pid = fork();

            if (pid == 0) { // Proceso hijo
                int start_row = p * rows_per_process;
                int end_row = start_row + rows_per_process;

                if (p == num_processes - 1) {
                    end_row += remaining_rows; // El último proceso maneja las filas restantes
                }

                for (int i = start_row; i < end_row; i++) {
                    for (int j = 0; j < N; j++) {
                        int resultado = 0;
                        for (int k = 0; k < N; k++) {
                            resultado += matriz_a[i * N + k] * matriz_b[k * N + j];
                        }
                        matriz_c[i * N + j] = resultado;
                    }
                }

                exit(0);
            } else if (pid < 0) { // Error en fork()
                fprintf(stderr, "Error en fork()\n");
                exit(1);
            }
        }
    }

    // Esperar a que todos los procesos hijos terminen
    for (int p = 0; p < num_processes; p++) {
        wait(NULL);
    }

    double end_time = omp_get_wtime();
    double cpu_time_used = end_time - start_time;

    if (verbose) {
        printf("Matriz A:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", matriz_a[i * N + j]);
            }
            printf("\n");
        }

        printf("\nMatriz B:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", matriz_b[i * N + j]);
            }
            printf("\n");
        }

        printf("\nMatriz resultado:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", matriz_c[i * N + j]);
            }
            printf("\n");
        }
    }

    // Liberar memoria compartida
    shmdt(matriz_a);
    shmdt(matriz_b);
    shmdt(matriz_c);
    shmctl(shmid_a, IPC_RMID, NULL);
    shmctl(shmid_b, IPC_RMID, NULL);
    shmctl(shmid_c, IPC_RMID, NULL);

    printf("\nTiempo de CPU: %lf segundos\n", cpu_time_used);

    return 0;
}

