#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define THRESHOLD 1000

int particionar(int vetor[], int baixo, int alto) {
    int pivo = vetor[alto];
    int i = baixo - 1;
    for (int j = baixo; j < alto; j++) {
        if (vetor[j] < pivo) {
            i++;
            int temp = vetor[i];
            vetor[i] = vetor[j];
            vetor[j] = temp;
        }
    }
    int temp = vetor[i + 1];
    vetor[i + 1] = vetor[alto];
    vetor[alto] = temp;
    return i + 1;
}

void quicksort_openmp(int vetor[], int baixo, int alto) {
    if (baixo < alto) {
        // Para subarrays pequenos, utiliza ordenação sequencial
        if (alto - baixo < THRESHOLD) {
            int pi = particionar(vetor, baixo, alto);
            quicksort_openmp(vetor, baixo, pi - 1);
            quicksort_openmp(vetor, pi + 1, alto);
        } else {
            int pi = particionar(vetor, baixo, alto);

            #pragma omp task shared(vetor) if((pi - 1) - baixo > THRESHOLD)
            quicksort_openmp(vetor, baixo, pi - 1);

            #pragma omp task shared(vetor) if(alto - (pi + 1) > THRESHOLD)
            quicksort_openmp(vetor, pi + 1, alto);

            #pragma omp taskwait
        }
    }
}

void preencher_vetor(int vetor[], int tamanho) {
    for (int i = 0; i < tamanho; i++)
        vetor[i] = rand() % 100000;
}

int main() {
    int tamanhos[] = {1000000, 5000000, 10000000, 25000000, 50000000, 75000000, 100000000, 250000000, 500000000};
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);
    double tempo_total = 0.0;

    srand(time(NULL));

    for (int i = 0; i < num_tamanhos; i++) {
        int tamanho = tamanhos[i];
        int *vetor = (int *)malloc(tamanho * sizeof(int));
        preencher_vetor(vetor, tamanho);

        double inicio = omp_get_wtime();

        #pragma omp parallel
        {
            #pragma omp single nowait
            {
                quicksort_openmp(vetor, 0, tamanho - 1);
            }
        }

        double fim = omp_get_wtime();
        double tempo_execucao = fim - inicio;
        tempo_total += tempo_execucao;

        printf("Tamanho: %d, Tempo: %.4f segundos\n", tamanho, tempo_execucao);
        free(vetor);
    }

    printf("\nTempo médio: %.4f segundos\n", tempo_total / num_tamanhos);
    return 0;
}
