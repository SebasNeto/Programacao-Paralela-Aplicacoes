#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define MAX_THREADS 16   // Número máximo de threads ativas
#define MIN_SIZE    10000 // Tamanho mínimo do subarray para criar nova thread

typedef struct {
    int *array;
    int low;
    int high;
} ThreadArgs;

pthread_mutex_t thread_count_mutex = PTHREAD_MUTEX_INITIALIZER;
int current_threads = 0;  // Número atual de threads criadas

// Função para trocar valores
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Seleção do pivô usando median-of-three
int median_of_three(int *array, int low, int high) {
    int mid = low + (high - low) / 2;
    if (array[low] > array[mid])
        swap(&array[low], &array[mid]);
    if (array[low] > array[high])
        swap(&array[low], &array[high]);
    if (array[mid] > array[high])
        swap(&array[mid], &array[high]);
    // Coloca o pivô em high-1
    swap(&array[mid], &array[high - 1]);
    return array[high - 1];
}

// Função de partição utilizando median-of-three quando possível
int partition(int *array, int low, int high) {
    // Se o subarray for muito pequeno, utiliza o último elemento como pivô
    if (high - low < 3) {
        int pivot = array[high];
        int i = low - 1;
        for (int j = low; j < high; j++) {
            if (array[j] < pivot) {
                i++;
                swap(&array[i], &array[j]);
            }
        }
        swap(&array[i + 1], &array[high]);
        return i + 1;
    }
    
    int pivot = median_of_three(array, low, high);
    int i = low;
    int j = high - 1;
    while (1) {
        while (array[++i] < pivot) {}
        while (array[--j] > pivot) {}
        if (i < j)
            swap(&array[i], &array[j]);
        else
            break;
    }
    swap(&array[i], &array[high - 1]); // Restaura o pivô
    return i;
}

// Quicksort sequencial com eliminação de recursão de cauda
void quicksort_sequential(int *array, int low, int high) {
    while (low < high) {
        // Se o subarray for pequeno, processa de forma sequencial
        if (high - low < MIN_SIZE) {
            int pi = partition(array, low, high);
            quicksort_sequential(array, low, pi - 1);
            low = pi + 1;
        } else {
            int pi = partition(array, low, high);
            quicksort_sequential(array, low, pi - 1);
            low = pi + 1;
        }
    }
}

// Função executada pelas threads: utiliza paralelismo com spawn único
void *quicksort_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int low = args->low;
    int high = args->high;
    int *array = args->array;
    free(args); // Libera os argumentos

    while (low < high) {
        // Se o subarray for pequeno, utiliza o quicksort sequencial
        if (high - low < MIN_SIZE) {
            quicksort_sequential(array, low, high);
            break;
        }
        
        int pi = partition(array, low, high);
        int spawn = 0;
        
        // Verifica se há disponibilidade para criar nova thread
        pthread_mutex_lock(&thread_count_mutex);
        if (current_threads < MAX_THREADS) {
            current_threads++;
            spawn = 1;
        }
        pthread_mutex_unlock(&thread_count_mutex);
        
        if (spawn) {
            // Cria thread para a partição esquerda
            ThreadArgs *new_args = malloc(sizeof(ThreadArgs));
            new_args->array = array;
            new_args->low = low;
            new_args->high = pi - 1;
            pthread_t thread;
            pthread_create(&thread, NULL, quicksort_thread, new_args);
            
            // Processa a partição direita na thread atual (eliminação de recursão de cauda)
            low = pi + 1;
            
            // Aguarda a conclusão da thread criada e atualiza o contador
            pthread_join(thread, NULL);
            pthread_mutex_lock(&thread_count_mutex);
            current_threads--;
            pthread_mutex_unlock(&thread_count_mutex);
        } else {
            // Se não for possível criar nova thread, processa a partição esquerda sequencialmente
            quicksort_sequential(array, low, pi - 1);
            low = pi + 1;
        }
    }
    return NULL;
}

void fill_array(int *array, int size) {
    for (int i = 0; i < size; i++)
        array[i] = rand();
}

int main() {
    int sizes[] = {10000000, 20000000, 30000000, 40000000, 50000000,
                   60000000, 70000000, 80000000, 90000000, 100000000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    double total_time = 0.0;

    srand(time(NULL));

    for (int i = 0; i < num_sizes; i++) {
        int size = sizes[i];
        int *array = malloc(size * sizeof(int));
        fill_array(array, size);

        // Cria os argumentos para a thread inicial
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->array = array;
        args->low = 0;
        args->high = size - 1;

        clock_t start = clock();
        pthread_t initial_thread;
        pthread_create(&initial_thread, NULL, quicksort_thread, args);
        pthread_join(initial_thread, NULL);
        clock_t end = clock();

        double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
        total_time += elapsed;
        printf("Tamanho: %d, Tempo: %.4f segundos\n", size, elapsed);
        free(array);
    }

    printf("\nTempo médio: %.4f segundos\n", total_time / num_sizes);
    return 0;
}
