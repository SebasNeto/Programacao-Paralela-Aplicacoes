#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define NUM_THREADS 16
#define BLOCK_SIZE 64  // Tamanho do bloco para otimização de cache

// Estrutura para passar dados para as threads
typedef struct {
    int n;
    double **A;
    double **B;
    double **C;
    int start_row;
    int end_row;
} ThreadData;

// Função para medir o tempo com precisão
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

// Função executada por cada thread para multiplicar parte da matriz
void *multiplicar_matrizes_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int n = data->n;
    double **A = data->A;
    double **B = data->B;
    double **C = data->C;
    int start_row = data->start_row;
    int end_row = data->end_row;

    for (int i = start_row; i < end_row; i += BLOCK_SIZE) {
        int limite_i = (i + BLOCK_SIZE > end_row) ? end_row : i + BLOCK_SIZE;
        for (int j = 0; j < n; j += BLOCK_SIZE) {
            int limite_j = (j + BLOCK_SIZE > n) ? n : j + BLOCK_SIZE;
            for (int k = 0; k < n; k += BLOCK_SIZE) {
                int limite_k = (k + BLOCK_SIZE > n) ? n : k + BLOCK_SIZE;
                for (int ii = i; ii < limite_i; ii++) {
                    for (int jj = j; jj < limite_j; jj++) {
                        double soma = 0.0;
                        for (int kk = k; kk < limite_k; kk++) {
                            soma += A[ii][kk] * B[kk][jj];
                        }
                        C[ii][jj] += soma;
                    }
                }
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
        double **C = (double **)malloc(n * sizeof(double *));
        for (int i = 0; i < n; i++) {
            A[i] = (double *)malloc(n * sizeof(double));
            B[i] = (double *)malloc(n * sizeof(double));
            C[i] = (double *)malloc(n * sizeof(double));
        }

        // Inicialização das matrizes A e B com valores aleatórios
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                A[i][j] = (double)rand() / RAND_MAX;
                B[i][j] = (double)rand() / RAND_MAX;
                C[i][j] = 0.0;
            }
        }

        // Medir tempo de execução
        double inicio_multiplicacao = get_time();

        pthread_t threads[NUM_THREADS];
        ThreadData thread_data[NUM_THREADS];
        int rows_per_thread = n / NUM_THREADS;

        for (int i = 0; i < NUM_THREADS; i++) {
            thread_data[i].n = n;
            thread_data[i].A = A;
            thread_data[i].B = B;
            thread_data[i].C = C;
            thread_data[i].start_row = i * rows_per_thread;
            thread_data[i].end_row = (i == NUM_THREADS - 1) ? n : (i + 1) * rows_per_thread;
            pthread_create(&threads[i], NULL, multiplicar_matrizes_thread, (void *)&thread_data[i]);
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        double fim_multiplicacao = get_time();
        double tempo_multiplicacao = fim_multiplicacao - inicio_multiplicacao;

        tempo_total += tempo_multiplicacao;

        printf("Tamanho da matriz: %d x %d\n", n, n);
        printf("Tempo de multiplicação: %.4f segundos\n\n", tempo_multiplicacao);

        // Liberação da memória alocada
        for (int i = 0; i < n; i++) {
            free(A[i]);
            free(B[i]);
            free(C[i]);
        }
        free(A);
        free(B);
        free(C);
    }

    double tempo_medio = tempo_total / num_tamanhos;
    printf("Tempo médio de execução: %.4f segundos\n", tempo_medio);

    return 0;
}
