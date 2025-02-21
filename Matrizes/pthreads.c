#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define NUM_THREADS 4

// Estrutura para passar dados para as threads
typedef struct {
    int n;
    double **A;
    double **B_transposta;
    double **C;
    int start_row;
    int end_row;
} ThreadData;

// Função para transpor uma matriz
void transpor_matriz(int n, double **matriz, double **transposta) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            transposta[j][i] = matriz[i][j];
        }
    }
}

// Função executada por cada thread para multiplicar parte da matriz
void *multiplicar_matrizes_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int n = data->n;
    double **A = data->A;
    double **B_transposta = data->B_transposta;
    double **C = data->C;
    int start_row = data->start_row;
    int end_row = data->end_row;

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B_transposta[j][k];
            }
        }
    }

    pthread_exit(NULL);
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
        transpor_matriz(n, B, B_transposta);
        clock_t fim_transposicao = clock();
        double tempo_transposicao = (double)(fim_transposicao - inicio_transposicao) / CLOCKS_PER_SEC;

        // Multiplicação das matrizes (A * B^T) com threads
        clock_t inicio_multiplicacao = clock();

        pthread_t threads[NUM_THREADS];
        ThreadData thread_data[NUM_THREADS];
        int rows_per_thread = n / NUM_THREADS;

        for (int i = 0; i < NUM_THREADS; i++) {
            thread_data[i].n = n;
            thread_data[i].A = A;
            thread_data[i].B_transposta = B_transposta;
            thread_data[i].C = C;
            thread_data[i].start_row = i * rows_per_thread;
            thread_data[i].end_row = (i + 1) * rows_per_thread;
            if (i == NUM_THREADS - 1) {
                thread_data[i].end_row = n; // A última thread pega as linhas restantes
            }
            pthread_create(&threads[i], NULL, multiplicar_matrizes_thread, (void *)&thread_data[i]);
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        clock_t fim_multiplicacao = clock();
        double tempo_multiplicacao = (double)(fim_multiplicacao - inicio_multiplicacao) / CLOCKS_PER_SEC;

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