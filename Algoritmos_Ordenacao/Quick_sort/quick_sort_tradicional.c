#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

void quicksort(int vetor[], int baixo, int alto) {
    if (baixo < alto) {
        int pi = particionar(vetor, baixo, alto);
        quicksort(vetor, baixo, pi - 1);
        quicksort(vetor, pi + 1, alto);
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

    srand(time(NULL));

    for (int i = 0; i < num_tamanhos; i++) {
        int tamanho = tamanhos[i];
        int *vetor = (int *)malloc(tamanho * sizeof(int));
        preencherVetor(vetor, tamanho);

        clock_t inicio = clock();
        quicksort(vetor, 0, tamanho - 1);
        clock_t fim = clock();

        double tempo_execucao = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
        tempo_total += tempo_execucao;

        printf("Tamanho: %d, Tempo: %.4f segundos\n", tamanho, tempo_execucao);
        free(vetor);
    }

    printf("\nTempo médio: %.4f segundos\n", tempo_total / num_tamanhos);
    return 0;
}




