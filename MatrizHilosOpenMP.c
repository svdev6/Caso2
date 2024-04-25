#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <omp.h>

#define MAX_THREADS 32

struct ThreadData {
    int **matriz_a;
    int **matriz_b;
    int **matriz_c;
    int N;
    int start_row;
    int end_row;
};

void multiplicar_matrices_seccion(int **matriz_a, int **matriz_b, int **matriz_c, int N, int start_row, int end_row) {
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            int resultado = 0;
            for (int k = 0; k < N; k++) {
                resultado += matriz_a[i][k] * matriz_b[k][j];
            }
            matriz_c[i][j] = resultado;
        }
    }
}

void *multiplicar_matrices_thread(void *arg) {
    struct ThreadData *data = (struct ThreadData *)arg;

    multiplicar_matrices_seccion(data->matriz_a, data->matriz_b, data->matriz_c, data->N, data->start_row, data->end_row);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s tamaño_de_matrices verbose(0 o 1) num_hilos\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int verbose = atoi(argv[2]);
    int num_threads = atoi(argv[3]);

    if (num_threads > MAX_THREADS) {
        printf("Número de hilos excede el máximo permitido (%d)\n", MAX_THREADS);
        return 1;
    }

    int **matriz_a = (int **)malloc(N * sizeof(int *));
    int **matriz_b = (int **)malloc(N * sizeof(int *));
    int **matriz_c = (int **)malloc(N * sizeof(int *));

    for (int i = 0; i < N; i++) {
        matriz_a[i] = (int *)malloc(N * sizeof(int));
        matriz_b[i] = (int *)malloc(N * sizeof(int));
        matriz_c[i] = (int *)malloc(N * sizeof(int));
    }

    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matriz_a[i][j] = rand() % 10 + 1;
            matriz_b[i][j] = rand() % 10 + 1;
        }
    }

    pthread_t threads[MAX_THREADS];
    struct ThreadData thread_data[MAX_THREADS];

    clock_t start_time = clock();

    int rows_per_thread = N / num_threads;
    int remaining_rows = N % num_threads;
    int current_row = 0;

    #pragma omp parallel num_threads(num_threads)
    {
        #pragma omp for
        for (int t = 0; t < num_threads; t++) {
            thread_data[t].matriz_a = matriz_a;
            thread_data[t].matriz_b = matriz_b;
            thread_data[t].matriz_c = matriz_c;
            thread_data[t].N = N;
            thread_data[t].start_row = current_row;
            thread_data[t].end_row = current_row + rows_per_thread + (t < remaining_rows ? 1 : 0);
            current_row = thread_data[t].end_row;

            multiplicar_matrices_seccion(thread_data[t].matriz_a, thread_data[t].matriz_b, thread_data[t].matriz_c, thread_data[t].N, thread_data[t].start_row, thread_data[t].end_row);
        }
    }

    clock_t end_time = clock();
    double cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    if (verbose) {
        printf("Matriz A:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", matriz_a[i][j]);
            }
            printf("\n");
        }

        printf("\nMatriz B:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", matriz_b[i][j]);
            }
            printf("\n");
        }

        printf("\nMatriz resultado:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", matriz_c[i][j]);
            }
            printf("\n");
        }
    }

    for (int i = 0; i < N; i++) {
        free(matriz_a[i]);
        free(matriz_b[i]);
        free(matriz_c[i]);
    }
    free(matriz_a);
    free(matriz_b);
    free(matriz_c);

    printf("\nTiempo de CPU: %lf segundos\n", cpu_time_used);

    return 0;
}

