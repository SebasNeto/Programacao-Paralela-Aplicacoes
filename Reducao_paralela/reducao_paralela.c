#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ITER 10  // Número de repetições para cada tamanho

void numAleatorios(int *vetor, size_t tamanho) {
    for (size_t i = 0; i < tamanho; i++) {
        vetor[i] = rand() % 10;
    }
}

int main() {
    // Semente para números aleatórios
    srand((unsigned) time(NULL));

    size_t tamanhos[] = {
        10000000, 20000000, 30000000, 40000000, 50000000,
        60000000, 70000000, 80000000, 90000000, 100000000
    };
    
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);

    double tempo_total = 0.0; 
    volatile long long soma_total_dummy = 0; 

    printf("Teste de Redução Sequencial\n");
    printf("Número de iterações por tamanho: %d\n\n", NUM_ITER);

    // Itera sobre cada tamanho de vetor
    for (int t = 0; t < num_tamanhos; t++) {
        size_t n = tamanhos[t];

        // Aloca o vetor dinamicamente
        int *vetor = malloc(n * sizeof(int));
        if (vetor == NULL) {
            fprintf(stderr, "Erro na alocação de memória para tamanho %zu\n", n);
            return EXIT_FAILURE;
        }

        // Preenche o vetor com números aleatórios
        numAleatorios(vetor, n);

        double tempo_medio = 0.0;

        long long soma = 0;
        for (size_t i = 0; i < n; i++)
            soma += vetor[i];          /* 1 × N leituras, 1 acumulador */

            

        for (int iter = 0; iter < NUM_ITER; iter++) {
            clock_t inicio = clock();

            // Soma os elementos do vetor sequencialmente
            long long soma_reducao = 0;
            for (size_t i = 0; i < n; i++) {
                soma_reducao += vetor[i];
            }

            clock_t fim = clock();
            double tempo_exec = (double)(fim - inicio) / CLOCKS_PER_SEC;
            tempo_medio += tempo_exec;

            soma_total_dummy += soma_reducao;
        }

        tempo_medio /= NUM_ITER;
        tempo_total += tempo_medio;
        printf("Tamanho do vetor: %zu -> Tempo médio: %lf segundos\n", n, tempo_medio);

        free(vetor);
    }

    double tempo_medio_geral = tempo_total / num_tamanhos;
    printf("\nTempo médio geral: %lf segundos\n", tempo_medio_geral);
    //printf("Soma total (dummy): %lld\n", soma_total_dummy);

    return 0;

    
}
