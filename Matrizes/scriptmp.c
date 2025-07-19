#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define BLOCK_SIZE 16  // Tamanho do bloco para otimização
#define CHUNK_SIZE 8   // Tamanho do chunk para balanceamento dinâmico


double* alocarMatriz(int n) {
    return (double*) malloc(n * n * sizeof(double));
}

// Libera a memória da matriz
void liberarMatriz(double *matriz) {
    free(matriz);
}

// Inicializa a matriz com valores aleatórios
void inicializarMatriz(double *matriz, int n) {
    for (int i = 0; i < n * n; i++) {
        matriz[i] = rand() % 10;
    }
}

// Função para multiplicação de matrizes usando OpenMP
void multMatriz(int n, double *A, double *B, double *C) {
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
    int tamanhos[] = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};
    int num_testes = sizeof(tamanhos) / sizeof(tamanhos[0]);
    double tempos[num_testes];

    printf("Número de threads disponíveis: %d\n\n", omp_get_max_threads());

    for (int t = 0; t < num_testes; t++) {
        int n = tamanhos[t];
        printf(" Criando matrizes de tamanho %d x %d ...\n", n, n);

        double *A = alocarMatriz(n);
        double *B = alocarMatriz(n);
        double *C = alocarMatriz(n);

        inicializarMatriz(A, n);
        inicializarMatriz(B, n);
        for (int i = 0; i < n * n; i++) C[i] = 0.0;

        printf(" Multiplicando matrizes de tamanho %d x %d ...\n", n, n);
        double inicio = omp_get_wtime();
        multMatriz(n, A, B, C);
        double fim = omp_get_wtime();

        tempos[t] = fim - inicio;
        printf(" Concluído: Tamanho %d x %d -> Tempo: %.4f segundos\n\n", n, n, tempos[t]);

        liberarMatriz(A);
        liberarMatriz(B);
        liberarMatriz(C);
    }

    double soma_tempos = 0;
    for (int i = 0; i < num_testes; i++) soma_tempos += tempos[i];
    printf("\n Média dos tempos de execução: %.4f segundos\n", soma_tempos / num_testes);
    return 0;
}
