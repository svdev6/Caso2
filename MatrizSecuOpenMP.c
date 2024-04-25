#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

void multiplicar_matrices(int **matriz_a, int **matriz_b, int **matriz_c, int N) {
    int i, j, k;

    #pragma omp parallel for private(i, j, k) shared(matriz_a, matriz_b, matriz_c)
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            matriz_c[i][j] = 0;
            for (k = 0; k < N; k++) {
                matriz_c[i][j] += matriz_a[i][k] * matriz_b[k][j];
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s tamaño_de_matrices verbose(0 o 1)\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int verbose = atoi(argv[2]);

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

    clock_t start_time = clock();

    multiplicar_matrices(matriz_a, matriz_b, matriz_c, N);

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

    printf("\nTiempo de CPU: %lf segundos\n", cpu_time_used);

    for (int i = 0; i < N; i++) {
        free(matriz_a[i]);
        free(matriz_b[i]);
        free(matriz_c[i]);
    }
    free(matriz_a);
    free(matriz_b);
    free(matriz_c);

    return 0;
}

