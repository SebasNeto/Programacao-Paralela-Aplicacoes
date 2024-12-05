#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/time.h>

void multiplicarMatrizes(int **A, int **B, int **C, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int **alocarMatriz(int N) {
    int **matriz = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        matriz[i] = (int *)malloc(N * sizeof(int));
    }
    return matriz;
}

void liberarMatriz(int **matriz, int N) {
    for (int i = 0; i < N; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

// void medirUsoDeMemoria() {
//     struct rusage uso;
//     getrusage(RUSAGE_SELF, &uso);
//     printf("Uso máximo de memória: %ld kilobytes\n", uso.ru_maxrss);
// }

void medirUsoDeCPUeMemoria(struct rusage *uso_inicio, struct rusage *uso_fim) {
    double utime = (uso_fim->ru_utime.tv_sec - uso_inicio->ru_utime.tv_sec) +
                   (uso_fim->ru_utime.tv_usec - uso_inicio->ru_utime.tv_usec) / 1e6;
    double stime = (uso_fim->ru_stime.tv_sec - uso_inicio->ru_stime.tv_sec) +
                   (uso_fim->ru_stime.tv_usec - uso_inicio->ru_stime.tv_usec) / 1e6;
    printf("Tempo de CPU em modo usuário: %f segundos\n", utime);
    printf("Tempo de CPU em modo sistema: %f segundos\n", stime);
    printf("Uso máximo de memória: %ld kilobytes\n", uso_fim->ru_maxrss);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <tamanho da matriz>\n", argv[0]);
        return -1;
    }

    int N = atoi(argv[1]); // Tamanho da matriz
    int **A = alocarMatriz(N);
    int **B = alocarMatriz(N);
    int **C = alocarMatriz(N);

    // Inicialização das matrizes A e B
    srand(time(NULL)); // Para valores aleatórios diferentes a cada execução
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = rand() % 100;
            B[i][j] = rand() % 100;
        }
    }

    struct rusage uso_inicio, uso_fim;
    getrusage(RUSAGE_SELF, &uso_inicio);

    struct timeval inicio, fim;
    gettimeofday(&inicio, NULL);

    multiplicarMatrizes(A, B, C, N);

    gettimeofday(&fim, NULL);
    getrusage(RUSAGE_SELF, &uso_fim);


    double tempo_execucao = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1e6;
    printf("Tempo de execução: %f segundos\n", tempo_execucao);

    medirUsoDeCPUeMemoria(&uso_inicio, &uso_fim);
    //medirUsoDeMemoria();

    liberarMatriz(A, N);
    liberarMatriz(B, N);
    liberarMatriz(C, N);

    return 0;
}

/////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_THREADS 4

int N = 1000; // Tamanho da matriz
int **A, **B, **C;

void *multiplicar(void *arg) {
    int id_thread = *(int *)arg;
    int tamanho_parte = N / MAX_THREADS;
    int linha_inicial = id_thread * tamanho_parte;
    int linha_final = (id_thread + 1) * tamanho_parte;

    for (int linha = linha_inicial; linha < linha_final; linha++) {
        for (int coluna = 0; coluna < N; coluna++) {
            C[linha][coluna] = 0;
            for (int k = 0; k < N; k++) {
                C[linha][coluna] += A[linha][k] * B[k][coluna];
            }
        }
    }
    pthread_exit(0);
}

int main() {
    pthread_t threads[MAX_THREADS];
    int id_threads[MAX_THREADS];

    // Alocação dinâmica das matrizes
    A = (int **)malloc(N * sizeof(int *));
    B = (int **)malloc(N * sizeof(int *));
    C = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        A[i] = (int *)malloc(N * sizeof(int));
        B[i] = (int *)malloc(N * sizeof(int));
        C[i] = (int *)malloc(N * sizeof(int));
    }

    // Inicialização das matrizes com valores aleatórios
    srand(time(NULL));
    for (int linha = 0; linha < N; linha++) {
        for (int coluna = 0; coluna < N; coluna++) {
            A[linha][coluna] = rand() % 10;
            B[linha][coluna] = rand() % 10;
        }
    }

    clock_t inicio = clock();

    // Criação das threads
    for (int i = 0; i < MAX_THREADS; i++) {
        id_threads[i] = i;
        pthread_create(&threads[i], NULL, multiplicar, (void *)&id_threads[i]);
    }

    // Espera pelas threads terminarem
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t fim = clock();

    double tempo_total = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Tempo de execução com threads: %f segundos\n", tempo_total);

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
