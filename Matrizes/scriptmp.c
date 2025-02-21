#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h> // Inclui a biblioteca OpenMP

// Função para transpor uma matriz
void transpor_matriz(int n, double **matriz, double **transposta) {
    #pragma omp parallel for collapse(2) // Paraleliza a transposição
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            transposta[j][i] = matriz[i][j];
        }
    }
}

// Função para multiplicar matrizes (A * B^T)
void multiplicar_matrizes(int n, double **A, double **B_transposta, double **C) {
    #pragma omp parallel for collapse(2) // Paraleliza os loops externos
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            #pragma omp simd // Usa instruções vetoriais (SIMD) para o loop interno
            for (int k = 0; k < n; k++) {
                sum += A[i][k] * B_transposta[j][k];
            }
            C[i][j] = sum;
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
        #pragma omp parallel for collapse(2) // Paraleliza a inicialização
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                A[i][j] = (double)rand() / RAND_MAX;
                B[i][j] = (double)rand() / RAND_MAX;
            }
        }

        // Transposição da matriz B
        double inicio_transposicao = omp_get_wtime();
        transpor_matriz(n, B, B_transposta);
        double fim_transposicao = omp_get_wtime();
        double tempo_transposicao = fim_transposicao - inicio_transposicao;

        // Multiplicação das matrizes (A * B^T)
        double inicio_multiplicacao = omp_get_wtime();
        multiplicar_matrizes(n, A, B_transposta, C);
        double fim_multiplicacao = omp_get_wtime();
        double tempo_multiplicacao = fim_multiplicacao - inicio_multiplicacao;

        // Tempo total (transposição + multiplicação)
        double tempo_total_execucao = tempo_transposicao + tempo_multiplicacao;
        tempo_total += tempo_total_execucao;

        printf("Tamanho da matriz: %d x %d\n", n, n);
        printf("Tempo de transposição: %.2f segundos\n", tempo_transposicao);
        printf("Tempo de multiplicação: %.2f segundos\n", tempo_multiplicacao);
        printf("Tempo total de execução: %.2f segundos\n\n", tempo_total_execucao);

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
    printf("Tempo médio de execução: %.2f segundos\n", tempo_medio);

    return 0;
}