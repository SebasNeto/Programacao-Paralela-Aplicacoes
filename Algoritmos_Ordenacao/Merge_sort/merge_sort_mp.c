#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MIN_SIZE 100000      // Tamanho mínimo para criar uma task
#define COPY_MIN_SIZE 100000 // Tamanho mínimo para paralelizar a cópia na mesclagem

// Função para mesclar dois subvetores usando o vetor auxiliar pré-alocado
void mesclar(int vetor[], int temp[], int esq, int meio, int dir) {
    int i = esq, j = meio + 1, k = esq;
    while (i <= meio && j <= dir) {
        if (vetor[i] <= vetor[j])
            temp[k++] = vetor[i++];
        else
            temp[k++] = vetor[j++];
    }
    while (i <= meio)
        temp[k++] = vetor[i++];
    while (j <= dir)
        temp[k++] = vetor[j++];
    
    // Copia de volta para o vetor original; paraleliza se o tamanho for grande
    int size = dir - esq + 1;
    if (size > COPY_MIN_SIZE) {
        #pragma omp parallel for
        for (i = esq; i <= dir; i++) {
            vetor[i] = temp[i];
        }
    } else {
        for (i = esq; i <= dir; i++) {
            vetor[i] = temp[i];
        }
    }
}

// Função recursiva de merge sort com OpenMP tasks
void merge_sort_openmp(int vetor[], int temp[], int esq, int dir) {
    if (esq < dir) {
        int meio = esq + (dir - esq) / 2;
        
        // Agrupa as tasks para as duas metades
        #pragma omp taskgroup
        {
            #pragma omp task if((meio - esq) > MIN_SIZE) shared(vetor, temp)
            {
                merge_sort_openmp(vetor, temp, esq, meio);
            }
            
            #pragma omp task if((dir - meio) > MIN_SIZE) shared(vetor, temp)
            {
                merge_sort_openmp(vetor, temp, meio + 1, dir);
            }
        }
        
        mesclar(vetor, temp, esq, meio, dir);
    }
}

void preencher_vetor(int vetor[], int tamanho) {
    for (int i = 0; i < tamanho; i++)
        vetor[i] = rand() % 100000;
}

int main() {
    int tamanhos[] = {1000000, 5000000, 10000000, 25000000, 50000000,
                      75000000, 100000000, 250000000, 500000000};
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);
    double tempo_total = 0.0;
    
    srand(42); // Garante reprodutibilidade

    for (int t = 0; t < num_tamanhos; t++) {
        int tamanho = tamanhos[t];
        int *vetor = (int *)malloc(tamanho * sizeof(int));
        int *temp = (int *)malloc(tamanho * sizeof(int));
        if (!vetor || !temp) {
            fprintf(stderr, "Erro de alocação para tamanho %d\n", tamanho);
            exit(1);
        }
        
        preencher_vetor(vetor, tamanho);
        double inicio = omp_get_wtime();
        
        #pragma omp parallel
        {
            #pragma omp single
            merge_sort_openmp(vetor, temp, 0, tamanho - 1);
        }
        
        double fim = omp_get_wtime();
        double tempo_execucao = fim - inicio;
        tempo_total += tempo_execucao;
        printf("Tamanho: %d, Tempo: %.4f segundos\n", tamanho, tempo_execucao);
        
        free(vetor);
        free(temp);
    }
    
    printf("\nTempo médio: %.4f segundos\n", tempo_total / num_tamanhos);
    return 0;
}
