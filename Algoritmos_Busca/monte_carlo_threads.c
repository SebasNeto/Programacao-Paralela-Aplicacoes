#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_ITER 10
#define NUM_SAMPLES 100000000LL  // Número total de amostras por iteração
#define NUM_THREADS 4           // Número de threads a serem utilizadas

// Estrutura para armazenar os dados passados para cada thread
typedef struct {
    long long samples;   // Número de amostras que a thread deverá processar
    unsigned int seed;   // Semente para o gerador rand_r
    long long count;     // Contagem local dos pontos dentro do círculo
} thread_data_t;

void* monte_carlo_thread(void* arg) {
    thread_data_t *data = (thread_data_t*) arg;
    long long local_count = 0;
    for (long long i = 0; i < data->samples; i++) {
        // Gera números aleatórios no intervalo [-1, 1]
        double x = ((double)rand_r(&data->seed) / RAND_MAX) * 2.0 - 1.0;
        double y = ((double)rand_r(&data->seed) / RAND_MAX) * 2.0 - 1.0;
        if (x * x + y * y <= 1.0)
            local_count++;
    }
    data->count = local_count;
    pthread_exit(NULL);
}

int main(void) {
    double total_time = 0.0;
    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data[NUM_THREADS];

    // Semente global para diferenciar as sementes de cada thread (poderia usar time(NULL))
    unsigned int global_seed = (unsigned int) time(NULL);

    for (int iter = 0; iter < NUM_ITER; iter++) {
        long long total_count = 0;
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        // Divide o número total de amostras entre as threads
        long long samples_per_thread = NUM_SAMPLES / NUM_THREADS;
        long long remainder = NUM_SAMPLES % NUM_THREADS;

        // Cria as threads
        for (int t = 0; t < NUM_THREADS; t++) {
            thread_data[t].samples = samples_per_thread + (t < remainder ? 1 : 0);
            // Cria sementes diferentes para cada thread (exemplo: soma do ID com a semente global)
            thread_data[t].seed = global_seed + t + iter;
            thread_data[t].count = 0;
            if (pthread_create(&threads[t], NULL, monte_carlo_thread, &thread_data[t]) != 0) {
                perror("pthread_create");
                exit(EXIT_FAILURE);
            }
        }
        
        // Aguarda o término de todas as threads e acumula os resultados
        for (int t = 0; t < NUM_THREADS; t++) {
            pthread_join(threads[t], NULL);
            total_count += thread_data[t].count;
        }
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        total_time += elapsed;
        
        double pi_est = 4.0 * (double)total_count / (double)NUM_SAMPLES;
        printf("Iteração %d: pi = %f, tempo = %f segundos\n", iter + 1, pi_est, elapsed);
    }
    
    printf("Tempo médio: %f segundos\n", total_time / NUM_ITER);
    return 0;
}
