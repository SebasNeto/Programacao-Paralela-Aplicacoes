#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void multiplicarMatrizes( int **A, int **B, int **C, int N){
    for (int linha =0; linha < N; linha++){
        for (int coluna = 0; coluna < N; coluna++){
            C[linha][coluna] = 0;
            for (int k = 0; k < N; k++){
                C[linha][coluna] += A[linha][k] * B[k][coluna];
            }
        }
    }
}

int main(){
    int N = 1000;
    int **A, **B, **C;

    A = (int **)malloc(N * sizeof(int *));
    B = (int **)malloc(N * sizeof(int *));
    C = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++){
        A[i] = (int *)malloc(N * sizeof(int));
        B[i] = (int *)malloc(N * sizeof(int));
        C[i] = (int *)malloc(N * sizeof(int));
    }

    srand(time(NULL));
    for (int linha = 0;  linha < N; linha++){
        for (int coluna = 0; coluna < N; coluna++){
            A[linha][coluna] = rand() % 10;
            B[linha][coluna] = rand() % 10;
        }
    }

    clock_t inicio = clock();
    multiplicarMatrizes(A, B, C, N);
    clock_t fim = clock();

    double tempoTotal = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Tempo de execução: %f segundos\n", tempoTotal);

    for (int i = 0; i < N; i++){
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }

    free(A);
    free(B);
    free(C);

    return 0;
}    