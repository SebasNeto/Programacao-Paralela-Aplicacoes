#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define BLOCK_SIZE 64  // Define o tamanho do bloco para otimização

// Função para multiplicação de matrizes usando OpenMP
void multiplicar_matrizes(int n, double **A, double **B, double **C) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < n; i += BLOCK_SIZE) {
        for (int j = 0; j < n; j += BLOCK_SIZE) {
            for (int k = 0; k < n; k += BLOCK_SIZE) {

                // Garantindo que os limites não ultrapassem o tamanho da matriz
                int limite_i = (i + BLOCK_SIZE > n) ? n : i + BLOCK_SIZE;
                int limite_j = (j + BLOCK_SIZE > n) ? n : j + BLOCK_SIZE;
                int limite_k = (k + BLOCK_SIZE > n) ? n : k + BLOCK_SIZE;

                for (int ii = i; ii < limite_i; ii++) {
                    for (int jj = j; jj < limite_j; jj++) {
                        for (int kk = k; kk < limite_k; kk++) {
                            C[ii][jj] += A[ii][kk] * B[kk][jj];
                        }
                    }
                }
            }
        }
    }
}

// Função para alocar dinamicamente uma matriz quadrada
double** alocar_matriz(int n) {
    double **matriz = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        matriz[i] = (double*)malloc(n * sizeof(double));
    }
    return matriz;
}

// Função para liberar a memória de uma matriz
void liberar_matriz(double **matriz, int n) {
    for (int i = 0; i < n; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

// Função para inicializar uma matriz com valores aleatórios
void inicializar_matriz(double **matriz, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matriz[i][j] = rand() % 10;
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
        printf("▶️ Criando matrizes de tamanho %d x %d ...\n", n, n);

        double **A = alocar_matriz(n);
        double **B = alocar_matriz(n);
        double **C = alocar_matriz(n);

        inicializar_matriz(A, n);
        inicializar_matriz(B, n);
        
        // Inicializar C com zeros
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                C[i][j] = 0.0;

        printf("🔄 Multiplicando matrizes de tamanho %d x %d ...\n", n, n);
        double inicio = omp_get_wtime();
        multiplicar_matrizes(n, A, B, C);
        double fim = omp_get_wtime();
        
        tempos[t] = fim - inicio;
        printf("✅ Concluído: Tamanho %d x %d -> Tempo: %.4f segundos\n\n", n, n, tempos[t]);

        liberar_matriz(A, n);
        liberar_matriz(B, n);
        liberar_matriz(C, n);
    }

    // Calcular a média do tempo de execução
    double soma_tempos = 0;
    for (int i = 0; i < num_testes; i++) {
        soma_tempos += tempos[i];
    }
    double media_tempo = soma_tempos / num_testes;

    printf("\n📊 Média dos tempos de execução: %.4f segundos\n", media_tempo);
    return 0;
}
