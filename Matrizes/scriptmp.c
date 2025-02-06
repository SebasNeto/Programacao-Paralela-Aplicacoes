#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

// gcc -DDEBUG scriptmp.c -o testemp -fopenmp

int main() {
    int N = 3000;
    int **A, **B, **C;

    // Alocação dinâmica das matrizes (mesmo que no código sequencial)
    // [Alocação e inicialização das matrizes A, B e C ]

    A = (int **)malloc(N * sizeof(int *));
    B = (int **)malloc(N * sizeof(int *));
    C = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        A[i] = (int *)malloc(N * sizeof(int));
        B[i] = (int *)malloc(N * sizeof(int));
        C[i] = (int *)malloc(N * sizeof(int));
    }

    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
        }
    }

    double start = omp_get_wtime();

    #pragma omp parallel for shared(A, B, C, N) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }

    double end = omp_get_wtime();
    printf("Tempo de execução com OpenMP: %f segundos\n", end - start);

    // Liberação da memória alocada
    for (int i = 0; i < N; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);

    return 0;
}