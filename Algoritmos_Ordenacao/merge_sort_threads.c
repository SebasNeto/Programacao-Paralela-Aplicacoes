#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define MAX_THREADS 16   // Número máximo de threads
#define MIN_SIZE    10000  // Tamanho mínimo do subarray para paralelizar

int num_threads = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int *vetor;
    int esq;
    int dir;
    int *temp;
} ThreadArgs;

void mesclar(int vetor[], int esq, int meio, int dir, int *temp) {
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
    for (i = esq; i <= dir; i++)
        vetor[i] = temp[i];
}

void merge_sort_sequential(int vetor[], int esq, int dir, int *temp) {
    if (esq < dir) {
        int meio = esq + (dir - esq) / 2;
        merge_sort_sequential(vetor, esq, meio, temp);
        merge_sort_sequential(vetor, meio + 1, dir, temp);
        mesclar(vetor, esq, meio, dir, temp);
    }
}

// Declaração antecipada para usar na função wrapper
void merge_sort_parallel(int vetor[], int esq, int dir, int *temp);

void *merge_sort_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *) arg;
    merge_sort_parallel(args->vetor, args->esq, args->dir, args->temp);
    free(args);
    return NULL;
}

// Função recursiva paralela que utiliza spawn para a metade esquerda
void merge_sort_parallel(int vetor[], int esq, int dir, int *temp) {
    if ((dir - esq + 1) < MIN_SIZE) {
        merge_sort_sequential(vetor, esq, dir, temp);
        return;
    }
    if (esq < dir) {
        int meio = esq + (dir - esq) / 2;
        int spawn_thread = 0;
        pthread_t thread_esq;
        
        pthread_mutex_lock(&mutex);
        if (num_threads < MAX_THREADS) {
            num_threads++;
            spawn_thread = 1;
        }
        pthread_mutex_unlock(&mutex);
        
        if (spawn_thread) {
            ThreadArgs *args_esq = malloc(sizeof(ThreadArgs));
            if (args_esq == NULL) { perror("malloc"); exit(EXIT_FAILURE); }
            args_esq->vetor = vetor;
            args_esq->esq = esq;
            args_esq->dir = meio;
            args_esq->temp = temp;
            pthread_create(&thread_esq, NULL, merge_sort_thread, (void *) args_esq);
        } else {
            merge_sort_parallel(vetor, esq, meio, temp);
        }
        
        merge_sort_parallel(vetor, meio + 1, dir, temp);
        
        if (spawn_thread) {
            pthread_join(thread_esq, NULL);
            pthread_mutex_lock(&mutex);
            num_threads--;
            pthread_mutex_unlock(&mutex);
        }
        mesclar(vetor, esq, meio, dir, temp);
    }
}

// Função inicial que prepara os argumentos e inicia o merge sort paralelo
void merge_sort_start(int vetor[], int n, int *temp) {
    ThreadArgs *args = malloc(sizeof(ThreadArgs));
    if (args == NULL) { perror("malloc"); exit(EXIT_FAILURE); }
    args->vetor = vetor;
    args->esq = 0;
    args->dir = n - 1;
    args->temp = temp;
    merge_sort_thread(args);  // Chamamos diretamente na thread principal
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
    
    srand(time(NULL));
    
    for (int i = 0; i < num_tamanhos; i++) {
        int tamanho = tamanhos[i];
        int *vetor = (int *)malloc(tamanho * sizeof(int));
        int *temp = (int *)malloc(tamanho * sizeof(int));
        if (!vetor || !temp) { perror("malloc"); exit(EXIT_FAILURE); }
        
        preencher_vetor(vetor, tamanho);
        
        clock_t inicio = clock();
        merge_sort_start(vetor, tamanho, temp);
        clock_t fim = clock();
        
        double tempo_execucao = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
        tempo_total += tempo_execucao;
        
        printf("Tamanho: %d, Tempo: %.4f segundos\n", tamanho, tempo_execucao);
        
        free(vetor);
        free(temp);
    }
    
    printf("\nTempo médio: %.4f segundos\n", tempo_total / num_tamanhos);
    return 0;
}
