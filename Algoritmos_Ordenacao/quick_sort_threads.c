#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define MAX_THREADS 16   // Número máximo de threads ativas
#define MIN_SIZE    10000  // Tamanho mínimo do subarray para criar nova thread

typedef struct {
    int *vetor;
    int baixo;
    int alto;
} ThreadArgs;

pthread_mutex_t thread_count_mutex = PTHREAD_MUTEX_INITIALIZER;
int current_threads = 0;  // Número atual de threads criadas

// Função de partição (Lomuto)
int particionar(int *vetor, int baixo, int alto) {
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

// Versão sequencial do quicksort
void quicksort_sequencial(int *vetor, int baixo, int alto) {
    if (baixo < alto) {
        int pi = particionar(vetor, baixo, alto);
        quicksort_sequencial(vetor, baixo, pi - 1);
        quicksort_sequencial(vetor, pi + 1, alto);
    }
}

// Função executada pelas threads
void *quicksort_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int baixo = args->baixo;
    int alto = args->alto;
    int *vetor = args->vetor;
    free(args); // Libera a memória dos argumentos

    if (baixo < alto) {
        // Se o subarray for pequeno, use o quicksort sequencial
        if ((alto - baixo) < MIN_SIZE) {
            quicksort_sequencial(vetor, baixo, alto);
            return NULL;
        }
        
        int pi = particionar(vetor, baixo, alto);
        pthread_t thread_esq = 0, thread_dir = 0;
        int criar_esq = 0, criar_dir = 0;

        // Prepara os argumentos para as partições esquerda e direita
        ThreadArgs *args_esq = malloc(sizeof(ThreadArgs));
        args_esq->vetor = vetor;
        args_esq->baixo = baixo;
        args_esq->alto = pi - 1;

        ThreadArgs *args_dir = malloc(sizeof(ThreadArgs));
        args_dir->vetor = vetor;
        args_dir->baixo = pi + 1;
        args_dir->alto = alto;

        // Tenta criar thread para a partição esquerda, se não ultrapassar o limite
        pthread_mutex_lock(&thread_count_mutex);
        if (current_threads < MAX_THREADS) {
            current_threads++;
            criar_esq = 1;
        }
        pthread_mutex_unlock(&thread_count_mutex);

        if (criar_esq) {
            pthread_create(&thread_esq, NULL, quicksort_thread, args_esq);
        } else {
            quicksort_sequencial(vetor, args_esq->baixo, args_esq->alto);
            free(args_esq);
        }

        // Tenta criar thread para a partição direita, se possível
        pthread_mutex_lock(&thread_count_mutex);
        if (current_threads < MAX_THREADS) {
            current_threads++;
            criar_dir = 1;
        }
        pthread_mutex_unlock(&thread_count_mutex);

        if (criar_dir) {
            pthread_create(&thread_dir, NULL, quicksort_thread, args_dir);
        } else {
            quicksort_sequencial(vetor, args_dir->baixo, args_dir->alto);
            free(args_dir);
        }

        // Aguarda a conclusão das threads criadas e atualiza o contador
        if (criar_esq) {
            pthread_join(thread_esq, NULL);
            pthread_mutex_lock(&thread_count_mutex);
            current_threads--;
            pthread_mutex_unlock(&thread_count_mutex);
        }
        if (criar_dir) {
            pthread_join(thread_dir, NULL);
            pthread_mutex_lock(&thread_count_mutex);
            current_threads--;
            pthread_mutex_unlock(&thread_count_mutex);
        }
    }
    return NULL;
}

void preencher_vetor(int *vetor, int tamanho) {
    for (int i = 0; i < tamanho; i++)
        vetor[i] = rand();
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
        preencher_vetor(vetor, tamanho);

        // Cria argumentos dinamicamente para a thread inicial
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->vetor = vetor;
        args->baixo = 0;
        args->alto = tamanho - 1;

        clock_t inicio = clock();
        pthread_t thread_inicial;
        // Inicia o quicksort em uma thread separada
        pthread_create(&thread_inicial, NULL, quicksort_thread, args);
        pthread_join(thread_inicial, NULL);
        clock_t fim = clock();

        double tempo_execucao = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
        tempo_total += tempo_execucao;
        printf("Tamanho: %d, Tempo: %.4f segundos\n", tamanho, tempo_execucao);
        free(vetor);
    }

    printf("\nTempo médio: %.4f segundos\n", tempo_total / num_tamanhos);
    return 0;
}
