#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define BLOCK_SIZE 2  // Tamanho do bloco para otimização
#define CHUNK_SIZE 1   // Tamanho do chunk para balanceamento dinâmico

// Aloca a matriz em um único bloco de memória
double* alocar_matriz(int n) {
    return (double*) malloc(n * n * sizeof(double));
}

// Libera a memória da matriz
void liberar_matriz(double *matriz) {
    free(matriz);
}

// Inicializa a matriz com valores aleatórios
void inicializar_matriz(double *matriz, int n) {
    for (int i = 0; i < n * n; i++) {
        matriz[i] = rand() % 10;
    }
}

// Função para multiplicação de matrizes usando OpenMP com melhor paralelismo
void multiplicar_matrizes(int n, double *A, double *B, double *C) {
    #pragma omp parallel for schedule(dynamic, CHUNK_SIZE)
    for (int i = 0; i < n; i += BLOCK_SIZE) {
        for (int j = 0; j < n; j += BLOCK_SIZE) {
            for (int k = 0; k < n; k += BLOCK_SIZE) {

                int limite_i = (i + BLOCK_SIZE > n) ? n : i + BLOCK_SIZE;
                int limite_j = (j + BLOCK_SIZE > n) ? n : j + BLOCK_SIZE;
                int limite_k = (k + BLOCK_SIZE > n) ? n : k + BLOCK_SIZE;

                for (int ii = i; ii < limite_i; ii++) {
                    for (int kk = k; kk < limite_k; kk++) {
                        double rA = A[ii * n + kk];  // Acessa um valor fixo de A
                        for (int jj = j; jj < limite_j; jj++) {
                            C[ii * n + jj] += rA * B[kk * n + jj]; // Evita acessos dispersos
                        }
                    }
                }
            }
        }
    }
}

int main() {
    int tamanho = 2; // Reduzido para um tamanho menor para teste
    
    printf("Número de threads disponíveis: %d\n\n", omp_get_max_threads());
    printf("▶️ Criando matrizes de tamanho %d x %d ...\n", tamanho, tamanho);

    double *A = alocar_matriz(tamanho);
    double *B = alocar_matriz(tamanho);
    double *C = alocar_matriz(tamanho);

    inicializar_matriz(A, tamanho);
    inicializar_matriz(B, tamanho);
    for (int i = 0; i < tamanho * tamanho; i++) C[i] = 0.0;

    printf("🔄 Multiplicando matrizes de tamanho %d x %d ...\n", tamanho, tamanho);
    double inicio = omp_get_wtime();
    multiplicar_matrizes(tamanho, A, B, C);
    double fim = omp_get_wtime();

    printf("✅ Concluído: Tamanho %d x %d -> Tempo: %.4f segundos\n\n", tamanho, tamanho, fim - inicio);

    // Exibir as matrizes para verificação
    printf("Matriz A:\n");
    for (int i = 0; i < tamanho; i++) {
        for (int j = 0; j < tamanho; j++) {
            printf("%.2f ", A[i * tamanho + j]);
        }
        printf("\n");
    }
    printf("\nMatriz B:\n");
    for (int i = 0; i < tamanho; i++) {
        for (int j = 0; j < tamanho; j++) {
            printf("%.2f ", B[i * tamanho + j]);
        }
        printf("\n");
    }
    printf("\nMatriz C (Resultado):\n");
    for (int i = 0; i < tamanho; i++) {
        for (int j = 0; j < tamanho; j++) {
            printf("%.2f ", C[i * tamanho + j]);
        }
        printf("\n");
    }

    liberar_matriz(A);
    liberar_matriz(B);
    liberar_matriz(C);

    return 0;
}