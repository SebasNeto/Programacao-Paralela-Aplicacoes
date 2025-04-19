// busca_binaria_tradicional.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_BUSCAS 100000
#define NUM_TAMANHOS 10
long tamanhos[] = {1000000, 5000000, 10000000, 25000000, 50000000, 75000000, 100000000, 250000000, 500000000,1000000000};

// Função de comparação para qsort
int comparar(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

// Implementação da busca binária tradicional
int busca_binaria(int arr[], int esquerda, int direita, int x) {
    while (esquerda <= direita) {
        int meio = esquerda + (direita - esquerda) / 2;
        
        if (arr[meio] == x)
            return meio;
        if (arr[meio] < x)
            esquerda = meio + 1;
        else
            direita = meio - 1;
    }
    return -1;
}

int main() {
    double tempo_total = 0;
    
    for (int t = 0; t < NUM_TAMANHOS; t++) {
        long TAMANHO_ARRAY = tamanhos[t];
        int *arr = (int*) malloc(TAMANHO_ARRAY * sizeof(int));
        int *buscas = (int*) malloc(NUM_BUSCAS * sizeof(int));
        int *resultados = (int*) malloc(NUM_BUSCAS * sizeof(int));
        
        srand(time(NULL));
        for (long i = 0; i < TAMANHO_ARRAY; i++) {
            arr[i] = i * 2;
        }
        
        for (int i = 0; i < NUM_BUSCAS; i++) {
            buscas[i] = rand() % (2 * TAMANHO_ARRAY);
        }
        
        qsort(arr, TAMANHO_ARRAY, sizeof(int), comparar);
        
        clock_t inicio = clock();
        for (int i = 0; i < NUM_BUSCAS; i++) {
            resultados[i] = busca_binaria(arr, 0, TAMANHO_ARRAY - 1, buscas[i]);
        }
        clock_t fim = clock();
        
        double tempo_execucao = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
        tempo_total += tempo_execucao;
        
        printf("Tamanho: %ld - Tempo da busca binária tradicional: %f segundos\n", TAMANHO_ARRAY, tempo_execucao);
        
        free(arr);
        free(buscas);
        free(resultados);
    }
    
    printf("Média do tempo de execução tradicional: %f segundos\n", tempo_total / NUM_TAMANHOS);
    return 0;
}