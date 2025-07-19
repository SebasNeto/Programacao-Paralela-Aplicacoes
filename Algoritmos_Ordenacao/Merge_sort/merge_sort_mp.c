#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MIN_SIZE 100000      
#define COPY_MIN_SIZE 100000 

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


void mergeSorOpenmp(int vetor[], int temp[], int esq, int dir) {
    if (esq < dir) {
        int meio = esq + (dir - esq) / 2;
        
        // Agrupa as tasks para as duas metades
        #pragma omp taskgroup
        {
            #pragma omp task if((meio - esq) > MIN_SIZE) shared(vetor, temp)
            {
                mergeSorOpenmp(vetor, temp, esq, meio);
            }
            
            #pragma omp task if((dir - meio) > MIN_SIZE) shared(vetor, temp)
            {
                mergeSorOpenmp(vetor, temp, meio + 1, dir);
            }
        }
        
        mesclar(vetor, temp, esq, meio, dir);
    }
}

void preencherVetor(int vetor[], int tamanho) {
    for (int i = 0; i < tamanho; i++)
        vetor[i] = rand() % 100000;
}

int main() {
    int tamanhos[] = {10000000, 20000000, 30000000, 40000000, 50000000, 60000000, 70000000, 80000000, 90000000, 100000000};
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
        
        preencherVetor(vetor, tamanho);
        double inicio = omp_get_wtime();
        
        #pragma omp parallel
        {
            #pragma omp single
            mergeSorOpenmp(vetor, temp, 0, tamanho - 1);
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
