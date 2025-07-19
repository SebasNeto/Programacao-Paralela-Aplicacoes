#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Função para transpor uma matriz
void transporMatriz(int n, double **matriz, double **transposta) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            transposta[j][i] = matriz[i][j];
        }
    }
}

// Função para multiplicar matrizes (A * B^T)
void multMatriz(int n, double **A, double **B_transposta, double **C) {
    for (int i = 0; i < n; i++) {           // percorre linhas de A
        for (int j = 0; j < n; j++) {       // percorre colunas de B ^T
            C[i][j] = 0.0;
            for (int k = 0; k < n; k++) {   //produto interno
                C[i][j] += A[i][k] * B_transposta[j][k];
            }
        }
    }
}

int main() {
    int tamanhos[] = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);
    double tempo_total = 0.0;

    for (int t = 0; t < num_tamanhos; t++) {
        int n = tamanhos[t];

        // Alocação dinâmica de memória para as matrizes
        double **A = (double **)malloc(n * sizeof(double *));
        double **B = (double **)malloc(n * sizeof(double *));
        double **B_transposta = (double **)malloc(n * sizeof(double *));
        double **C = (double **)malloc(n * sizeof(double *));
        for (int i = 0; i < n; i++) {
            A[i] = (double *)malloc(n * sizeof(double));
            B[i] = (double *)malloc(n * sizeof(double));
            B_transposta[i] = (double *)malloc(n * sizeof(double));
            C[i] = (double *)malloc(n * sizeof(double));
        }

        // Inicialização das matrizes A e B com valores aleatórios
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                A[i][j] = (double)rand() / RAND_MAX;
                B[i][j] = (double)rand() / RAND_MAX;
            }
        }

        // Transposição da matriz B
        clock_t inicio_transposicao = clock();
        transporMatriz(n, B, B_transposta);
        clock_t fim_transposicao = clock();
        double tempo_transposicao = (double)(fim_transposicao - inicio_transposicao) / CLOCKS_PER_SEC;

        // Multiplicação das matrizes (A * B^T)
        clock_t inicio_multiplicacao = clock();
        multMatriz(n, A, B_transposta, C);
        clock_t fim_multiplicacao = clock();
        double tempo_multiplicacao = (double)(fim_multiplicacao - inicio_multiplicacao) / CLOCKS_PER_SEC;

        // Tempo total (transposição + multiplicação)
        double tempo_total_execucao = tempo_transposicao + tempo_multiplicacao;
        tempo_total += tempo_total_execucao;

        printf("Tamanho da matriz: %d x %d\n", n, n);
        printf("Tempo de transposição: %.4f segundos\n", tempo_transposicao);
        printf("Tempo de multiplicação: %.4f segundos\n", tempo_multiplicacao);
        printf("Tempo total de execução: %.4f segundos\n\n", tempo_total_execucao);

        // Liberação da memória alocada
        for (int i = 0; i < n; i++) {
            free(A[i]);
            free(B[i]);
            free(B_transposta[i]);
            free(C[i]);
        }
        free(A);
        free(B);
        free(B_transposta);
        free(C);
    }

    double tempo_medio = tempo_total / num_tamanhos;
    printf("Tempo médio de execução: %.4f segundos\n", tempo_medio);

    return 0;
}






