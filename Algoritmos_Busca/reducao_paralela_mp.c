#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define NUM_ITER 10  // Número de repetições para cada tamanho

// Função para preencher o vetor com números aleatórios (valores entre 0 e 9)
void gerar_vetor_aleatorio(int *vetor, size_t tamanho) {
    for (size_t i = 0; i < tamanho; i++) {
        vetor[i] = rand() % 10;
    }
}

int main() {
    // Lista dos tamanhos de vetor para teste (de 10M a 100M elementos)
    size_t tamanhos[] = {10000000, 20000000, 30000000, 40000000, 50000000,
                         60000000, 70000000, 80000000, 90000000, 100000000};
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);
    double tempo_total = 0.0;
    volatile long long soma_total_dummy = 0;  // Variável dummy para evitar otimização

    // Semente para números aleatórios
    srand((unsigned) time(NULL));

    printf("Benchmark de Redução Paralela com OpenMP (versão aprimorada)\n");
    printf("Número de iterações por tamanho: %d\n\n", NUM_ITER);

    for (int t = 0; t < num_tamanhos; t++) {
        size_t n = tamanhos[t];

        // Aloca memória alinhada a 64 bytes para melhorar o desempenho de cache/vetorização
        int *vetor = NULL;
        if (posix_memalign((void**)&vetor, 64, n * sizeof(int)) != 0) {
            fprintf(stderr, "Erro na alocação de memória para tamanho %zu\n", n);
            return EXIT_FAILURE;
        }
        
        gerar_vetor_aleatorio(vetor, n);

        double tempo_medio = 0.0;

        // Executa NUM_ITER iterações para cada tamanho e acumula o tempo médio
        for (int iter = 0; iter < NUM_ITER; iter++) {
            double inicio = omp_get_wtime();

            long long soma_reducao = 0;
            // Redução paralela com schedule estático para balancear as iterações
            #pragma omp parallel for reduction(+:soma_reducao) schedule(static)
            for (size_t i = 0; i < n; i++) {
                soma_reducao += vetor[i];
            }

            double fim = omp_get_wtime();
            tempo_medio += (fim - inicio);
            
            // Acumula a soma para evitar que o compilador otimize a operação
            soma_total_dummy += soma_reducao;
        }

        tempo_medio /= NUM_ITER;
        tempo_total += tempo_medio;
        printf("Tamanho do vetor: %zu -> Tempo médio: %lf segundos\n", n, tempo_medio);

        free(vetor);
    }

    double tempo_medio_geral = tempo_total / num_tamanhos;
    printf("\nTempo médio geral: %lf segundos\n", tempo_medio_geral);
    // Impressão dummy para evitar remoção de código pelo compilador
    printf("Soma total (dummy): %lld\n", soma_total_dummy);

    return 0;
}
