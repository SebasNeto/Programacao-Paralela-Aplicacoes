//reducao paralela com threads v2
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#define NUM_ITER 10        // Número de repetições para cada tamanho
#define FIXED_THREADS 4    // Número fixo de threads

// Estrutura para passar dados para cada thread
typedef struct {
    int *restrict array;
    size_t start;
    size_t end;
    long long partial_sum;
} ThreadData;

// Função que cada thread executará para calcular a soma parcial
void *thread_func(void *arg) {
    ThreadData *data = (ThreadData *) arg;
    long long sum = 0;
    for (size_t i = data->start; i < data->end; i++) {
        sum += data->array[i];
    }
    data->partial_sum = sum;
    return NULL;
}

// Função para preencher o vetor com números aleatórios entre 0 e 9
void gerar_vetor_aleatorio(int *restrict array, size_t n) {
    for (size_t i = 0; i < n; i++) {
        array[i] = rand() % 10;
    }
}

int main() {
    // Lista dos tamanhos de vetor para teste (de 10M a 100M elementos)
    size_t tamanhos[] = {10000000, 20000000, 30000000, 40000000, 50000000,
                         60000000, 70000000, 80000000, 90000000, 100000000};
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);

    int num_threads = FIXED_THREADS;  // Número fixo de threads

    double tempo_total = 0.0;
    volatile long long soma_total_dummy = 0;  // Variável dummy para evitar otimizações

    srand((unsigned) time(NULL));

    printf("Benchmark de Redução Paralela com POSIX Threads (Número fixo de threads: %d)\n", num_threads);
    printf("Número de iterações por tamanho: %d\n\n", NUM_ITER);

    for (int t = 0; t < num_tamanhos; t++) {
        size_t n = tamanhos[t];

        // Aloca memória alinhada a 64 bytes
        int *restrict array = NULL;
        if (posix_memalign((void**)&array, 64, n * sizeof(int)) != 0) {
            fprintf(stderr, "Erro na alocação de memória para tamanho %zu\n", n);
            return EXIT_FAILURE;
        }

        gerar_vetor_aleatorio(array, n);
        double tempo_medio = 0.0;

        for (int iter = 0; iter < NUM_ITER; iter++) {
            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);

            pthread_t threads[num_threads];
            ThreadData thread_data[num_threads];

            // Divide o vetor entre as threads
            size_t chunk = n / num_threads;
            size_t remainder = n % num_threads;
            size_t start_index = 0;
            for (int i = 0; i < num_threads; i++) {
                thread_data[i].array = array;
                thread_data[i].start = start_index;
                size_t extra = (i < remainder) ? 1 : 0;
                thread_data[i].end = start_index + chunk + extra;
                thread_data[i].partial_sum = 0;
                start_index = thread_data[i].end;
                if (pthread_create(&threads[i], NULL, thread_func, &thread_data[i]) != 0) {
                    fprintf(stderr, "Erro ao criar thread %d\n", i);
                    free(array);
                    return EXIT_FAILURE;
                }
            }

            // Aguarda o término de todas as threads e acumula o resultado
            long long soma_reducao = 0;
            for (int i = 0; i < num_threads; i++) {
                pthread_join(threads[i], NULL);
                soma_reducao += thread_data[i].partial_sum;
            }

            clock_gettime(CLOCK_MONOTONIC, &end);
            double tempo_exec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            tempo_medio += tempo_exec;

            soma_total_dummy += soma_reducao;
        }

        tempo_medio /= NUM_ITER;
        tempo_total += tempo_medio;
        printf("Tamanho do vetor: %zu -> Tempo médio: %lf segundos\n", n, tempo_medio);

        free(array);
    }

    double tempo_medio_geral = tempo_total / num_tamanhos;
    printf("\nTempo médio geral: %lf segundos\n", tempo_medio_geral);
    printf("Soma total (dummy): %lld\n", soma_total_dummy);

    return 0;
}
