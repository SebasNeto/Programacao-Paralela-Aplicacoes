#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void mesclar(int vetor[], int esq, int meio, int dir) {
    int i, j, k;
    int tam_esq = meio - esq + 1;
    int tam_dir = dir - meio;

    int *esquerda = (int *)malloc(tam_esq * sizeof(int));
    int *direita = (int *)malloc(tam_dir * sizeof(int));

    for (i = 0; i < tam_esq; i++) esquerda[i] = vetor[esq + i];
    for (j = 0; j < tam_dir; j++) direita[j] = vetor[meio + 1 + j];

    i = 0, j = 0, k = esq;
    while (i < tam_esq && j < tam_dir) {
        if (esquerda[i] <= direita[j]) vetor[k++] = esquerda[i++];
        else vetor[k++] = direita[j++];
    }

    while (i < tam_esq) vetor[k++] = esquerda[i++];
    while (j < tam_dir) vetor[k++] = direita[j++];

    free(esquerda);
    free(direita);
}

void merge_sort(int vetor[], int esq, int dir) {
    if (esq < dir) {
        int meio = esq + (dir - esq) / 2;
        merge_sort(vetor, esq, meio);
        merge_sort(vetor, meio + 1, dir);
        mesclar(vetor, esq, meio, dir);
    }
}

void preencher_vetor(int vetor[], int tamanho) {
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
        preencher_vetor(vetor, tamanho);

        clock_t inicio = clock();
        merge_sort(vetor, 0, tamanho - 1);
        clock_t fim = clock();

        double tempo_execucao = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
        tempo_total += tempo_execucao;

        printf("Tamanho: %d, Tempo: %.4f segundos\n", tamanho, tempo_execucao);
        free(vetor);
    }

    printf("\nTempo médio: %.4f segundos\n", tempo_total / num_tamanhos);
    return 0;
}
