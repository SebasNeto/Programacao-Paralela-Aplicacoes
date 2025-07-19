#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>


#define MAX_THREADS 16   // Número máximo de threads
#define MIN_SIZE    10000  // Tamanho mínimo do subarray para paralelizar

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int num_threads = 0;

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

void mergeSorSeq(int vetor[], int esq, int dir, int *temp) {
    if (esq < dir) {
        int meio = esq + (dir - esq) / 2;
        mergeSorSeq(vetor, esq, meio, temp);
        mergeSorSeq(vetor, meio + 1, dir, temp);
        mesclar(vetor, esq, meio, dir, temp);
    }
}

void *mergeSorThread(void *arg) {
    ThreadArgs *args = (ThreadArgs *) arg;
    mergeSorSeq(args->vetor, args->esq, args->dir, args->temp);
    free(args);
    return NULL;
}


void mergeSorParalelo(int vetor[], int esq, int dir, int *temp) {
    if ((dir - esq + 1) < MIN_SIZE) {
        mergeSorSeq(vetor, esq, dir, temp);
        return;
    }
    
    if (esq < dir) {
        int meio = esq + (dir - esq) / 2;
        pthread_t thread_esq, thread_dir;
        int use_thread_esq = 0, use_thread_dir = 0;

        pthread_mutex_lock(&mutex);
        if (num_threads < MAX_THREADS) {
            num_threads++;
            use_thread_esq = 1;
        }
        if (num_threads < MAX_THREADS) {
            num_threads++;
            use_thread_dir = 1;
        }
        pthread_mutex_unlock(&mutex);

        if (use_thread_esq) {
            ThreadArgs *args_esq = malloc(sizeof(ThreadArgs));
            args_esq->vetor = vetor;
            args_esq->esq = esq;
            args_esq->dir = meio;
            args_esq->temp = temp;
            pthread_create(&thread_esq, NULL, mergeSorThread, (void *) args_esq);
        } else {
            mergeSorParalelo(vetor, esq, meio, temp);
        }

        if (use_thread_dir) {
            ThreadArgs *args_dir = malloc(sizeof(ThreadArgs));
            args_dir->vetor = vetor;
            args_dir->esq = meio + 1;
            args_dir->dir = dir;
            args_dir->temp = temp;
            pthread_create(&thread_dir, NULL, mergeSorThread, (void *) args_dir);
        } else {
            mergeSorParalelo(vetor, meio + 1, dir, temp);
        }

        if (use_thread_esq) {
            pthread_join(thread_esq, NULL);
            pthread_mutex_lock(&mutex);
            num_threads--;
            pthread_mutex_unlock(&mutex);
        }

        if (use_thread_dir) {
            pthread_join(thread_dir, NULL);
            pthread_mutex_lock(&mutex);
            num_threads--;
            pthread_mutex_unlock(&mutex);
        }

        mesclar(vetor, esq, meio, dir, temp);
    }
}

void startMerge(int vetor[], int n, int *temp) {
    mergeSorParalelo(vetor, 0, n - 1, temp);
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
        int *temp = (int *)malloc(tamanho * sizeof(int));
        if (!vetor || !temp) { perror("malloc"); exit(EXIT_FAILURE); }
        
        preencherVetor(vetor, tamanho);
        
        clock_t inicio = clock();
        startMerge(vetor, tamanho, temp);
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
