#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h> // para sysconf

#define NUM_ITER 10  // Número de repetições para cada tamanho

// Estrutura para passar dados para cada thread
typedef struct {
    int * restrict array;
    size_t start;
    size_t end;
    long long partial_sum;
} ThreadData;

// Função que cada thread executará para calcular a soma parcial
void *thread_func(void *arg) {
    ThreadData *data = (ThreadData*) arg;
    long long sum = 0;
    for (size_t i = data->start; i < data->end; i++) {
        sum += data->array[i];
    }
    data->partial_sum = sum;
    return NULL;
}

// Função para preencher o vetor com números aleatórios entre 0 e 9
void gerar_vetor_aleatorio(int * restrict array, size_t n) {
    for (size_t i = 0; i < n; i++) {
        array[i] = rand() % 10;
    }
}

int main() {
    // Lista dos tamanhos de vetor para teste (de 10M a 100M elementos)
    size_t tamanhos[] = {10000000, 20000000, 30000000, 40000000, 50000000,
                         60000000, 70000000, 80000000, 90000000, 100000000};
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);

    // Obtém o número de threads disponíveis no sistema
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads < 1)
        num_threads = 1;

    double tempo_total = 0.0;
    volatile long long soma_total_dummy = 0;  // Variável dummy para evitar otimização

    srand((unsigned) time(NULL));

    printf("Benchmark de Redução Paralela com POSIX Threads\n");
    printf("Número de iterações por tamanho: %d\n", NUM_ITER);
    printf("Número de threads: %d\n\n", num_threads);

    for (int t = 0; t < num_tamanhos; t++) {
        size_t n = tamanhos[t];

        // Aloca memória alinhada a 64 bytes para melhor desempenho de cache e vetorização
        int * restrict array = NULL;
        if (posix_memalign((void**)&array, 64, n * sizeof(int)) != 0) {
            fprintf(stderr, "Erro na alocação de memória para tamanho %zu\n", n);
            return EXIT_FAILURE;
        }

        gerar_vetor_aleatorio(array, n);
        double tempo_medio = 0.0;

        // Executa NUM_ITER iterações para cada tamanho
        for (int iter = 0; iter < NUM_ITER; iter++) {
            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);

            // Cria arrays para threads e seus dados
            pthread_t threads[num_threads];
            ThreadData thread_data[num_threads];

            // Divide o vetor entre as threads
            size_t chunk = n / num_threads;
            size_t remainder = n % num_threads;
            size_t start_index = 0;
            for (int i = 0; i < num_threads; i++) {
                thread_data[i].array = array;
                thread_data[i].start = start_index;
                // Distribui o resto entre as primeiras threads
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

            // Junta todas as threads e soma os resultados parciais
            long long soma_reducao = 0;
            for (int i = 0; i < num_threads; i++) {
                pthread_join(threads[i], NULL);
                soma_reducao += thread_data[i].partial_sum;
            }

            clock_gettime(CLOCK_MONOTONIC, &end);
            double tempo_exec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            tempo_medio += tempo_exec;

            // Acumula a soma para evitar que o compilador otimize a operação
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
