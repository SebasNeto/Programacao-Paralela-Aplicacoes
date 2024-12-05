#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

//gcc -pthread -o threads script_pthreads.c

#define MAX_THREADS 4

int N = 2000; // Tamanho da matriz
int *A, *B, *C;
int *B_transposta;

void *multiplicar(void *arg) {
    int id_thread = *(int *)arg;
    int linhas_por_thread = (N + MAX_THREADS - 1) / MAX_THREADS;
    int linha_inicial = id_thread * linhas_por_thread;
    int linha_final = (id_thread + 1) * linhas_por_thread;
    if (linha_final > N) linha_final = N;

    for (int i = linha_inicial; i < linha_final; i++) {
        for (int j = 0; j < N; j++) {
            int soma = 0;
            for (int k = 0; k < N; k++) {
                soma += A[i * N + k] * B_transposta[j * N + k];
            }
            C[i * N + j] = soma;
        }
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[MAX_THREADS];
    int id_threads[MAX_THREADS];

    // Alocação contígua das matrizes
    A = (int *)malloc(N * N * sizeof(int));
    B = (int *)malloc(N * N * sizeof(int));
    C = (int *)malloc(N * N * sizeof(int));
    B_transposta = (int *)malloc(N * N * sizeof(int));

    if (A == NULL || B == NULL || C == NULL || B_transposta == NULL) {
        fprintf(stderr, "Erro ao alocar memória.\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    // Inicialização das matrizes A e B
    for (int i = 0; i < N * N; i++) {
        A[i] = rand() % 10;
        B[i] = rand() % 10;
        C[i] = 0;
    }

    // Transpor a matriz B para melhorar a localidade de memória
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            B_transposta[j * N + i] = B[i * N + j];
        }
    }

    clock_t inicio = clock();

    // Criação das threads
    for (int i = 0; i < MAX_THREADS; i++) {
        id_threads[i] = i;
        int rc = pthread_create(&threads[i], NULL, multiplicar, (void *)&id_threads[i]);
        if (rc) {
            fprintf(stderr, "Erro ao criar thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // Espera pelas threads terminarem
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t fim = clock();

    double tempo_total = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Tempo de execução com threads: %f segundos\n", tempo_total);

    // Liberação da memória alocada
    free(A);
    free(B);
    free(C);
    free(B_transposta);

    return 0;
}
