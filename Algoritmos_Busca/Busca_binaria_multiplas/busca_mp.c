// busca_binaria_paralela_otimizado.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define NUM_BUSCAS 100000
#define NUM_THREADS 16
#define NUM_TAMANHOS 9
long tamanhos[] = {1000000, 5000000, 10000000, 25000000, 50000000, 75000000, 100000000, 250000000, 500000000};
//correct
// Implementação da busca binária
int busca_binaria(const int arr[], long esquerda, long direita, int x) {
    while (esquerda <= direita) {
        long meio = esquerda + (direita - esquerda) / 2;
        if (arr[meio] == x)
            return (int)meio;
        else if (arr[meio] < x)
            esquerda = meio + 1;
        else
            direita = meio - 1;
    }
    return -1;
}

int main() {
    double tempo_total = 0.0;
    srand((unsigned)time(NULL));
    omp_set_num_threads(NUM_THREADS);
    
    for (int t = 0; t < NUM_TAMANHOS; t++) {
        long TAMANHO_ARRAY = tamanhos[t];
        int *arr = (int*) malloc(TAMANHO_ARRAY * sizeof(int));
        int *buscas = (int*) malloc(NUM_BUSCAS * sizeof(int));
        int *resultados = (int*) malloc(NUM_BUSCAS * sizeof(int));
        
        if (!arr || !buscas || !resultados) {
            fprintf(stderr, "Erro na alocacao de memoria\n");
            exit(EXIT_FAILURE);
        }
        
        // Preenche o array com valores já ordenados
        for (long i = 0; i < TAMANHO_ARRAY; i++) {
            arr[i] = i * 2;
        }
        
        // Gera valores aleatórios para as buscas
        for (int i = 0; i < NUM_BUSCAS; i++) {
            buscas[i] = rand() % (2 * TAMANHO_ARRAY);
        }
        
        // Se o array já está ordenado, não é necessário chamar qsort.
        // Caso necessário, descomente a linha abaixo:
        // qsort(arr, TAMANHO_ARRAY, sizeof(int), comparar);
        
        double inicio = omp_get_wtime();
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < NUM_BUSCAS; i++) {
            resultados[i] = busca_binaria(arr, 0, TAMANHO_ARRAY - 1, buscas[i]);
        }
        double fim = omp_get_wtime();
        
        double tempo_execucao = fim - inicio;
        tempo_total += tempo_execucao;
        
        printf("Tamanho: %ld - Tempo da busca binaria paralela: %f segundos\n", TAMANHO_ARRAY, tempo_execucao);
        
        free(arr);
        free(buscas);
        free(resultados);
    }
    
    printf("Media do tempo de execucao paralela: %f segundos\n", tempo_total / NUM_TAMANHOS);
    return 0;
}
