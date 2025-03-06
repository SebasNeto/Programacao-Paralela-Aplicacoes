//monte carlo original em julia

using Random, Statistics, Base.Threads

const NUM_ITER = 10
const NUM_SAMPLES = 100_000_000  # Número total de amostras por iteração

# Função que processa um número específico de amostras usando um gerador RNG fornecido
function monte_carlo_pi_thread(samples::Int, rng::AbstractRNG)
    count = 0
    for _ in 1:samples
        x = rand(rng) * 2.0 - 1.0
        y = rand(rng) * 2.0 - 1.0
        if x*x + y*y <= 1.0
            count += 1
        end
    end
    return count
end

# Função paralela que distribui as amostras entre as threads e retorna a estimativa de π
function monte_carlo_pi_parallel(samples_total::Int)
    nthreads = Threads.nthreads()
    samples_per_thread = div(samples_total, nthreads)
    remainder = samples_total % nthreads
    counts = zeros(Int, nthreads)
    
    @threads for tid in 1:nthreads
        # Cada thread processa um número de amostras ajustado para distribuir o resto
        samples = samples_per_thread + (tid <= remainder ? 1 : 0)
        # Inicializa um gerador de números aleatórios específico para a thread
        rng = MersenneTwister(Threads.threadid() + 1234 + tid)
        counts[tid] = monte_carlo_pi_thread(samples, rng)
    end
    total_count = sum(counts)
    return 4.0 * total_count / samples_total
end

# Função principal que executa várias iterações, mede o tempo e calcula a média
function main()
    times = Float64[]
    for iter in 1:NUM_ITER
        t_start = time()
        pi_est = monte_carlo_pi_parallel(NUM_SAMPLES)
        t_end = time()
        elapsed = t_end - t_start
        push!(times, elapsed)
        println("Iteração $iter: pi = $pi_est, tempo = $(elapsed) segundos")
    end
    println("Tempo médio: $(mean(times)) segundos")
end

main()

//monte carlo original em openmp

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define NUM_ITER 10
#define NUM_SAMPLES 100000000  

int main(void) {
    double total_time = 0.0;
    
    srand48(time(NULL));
    
    for (int iter = 0; iter < NUM_ITER; iter++) {
        long long count = 0;
        double start_time = omp_get_wtime();
        
        #pragma omp parallel reduction(+:count)
        {

            unsigned int seed = omp_get_thread_num() + time(NULL);
            #pragma omp for
            for (long long i = 0; i < NUM_SAMPLES; i++) {
                double x = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
                double y = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
                if (x*x + y*y <= 1.0)
                    count++;
            }
        }
        
        double end_time = omp_get_wtime();
        double elapsed = end_time - start_time;
        total_time += elapsed;
        
        double pi = 4.0 * (double)count / (double)NUM_SAMPLES;
        printf("Iteração %d: pi = %f, tempo = %f segundos\n", iter+1, pi, elapsed);
    }
    
    printf("Tempo médio: %f segundos\n", total_time / NUM_ITER);
    return 0;
}

//monte carlo original com threads

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


//monte carlo original em c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ITER 10
#define NUM_SAMPLES 100000000 

double random_double() {
    return (double)rand() / (double)RAND_MAX;
}

int main(void) {
    double total_time = 0.0;
    
    // Inicializa a semente do gerador de números aleatórios
    srand(time(NULL));
    
    for (int iter = 0; iter < NUM_ITER; iter++) {
        long long count = 0;
        struct timespec start, end;
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        // Geração das amostras e contagem dos pontos dentro do círculo
        for (long long i = 0; i < NUM_SAMPLES; i++) {
            double x = random_double() * 2.0 - 1.0;
            double y = random_double() * 2.0 - 1.0;
            if (x*x + y*y <= 1.0)
                count++;
        }
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed = (end.tv_sec - start.tv_sec) +
                         (end.tv_nsec - start.tv_nsec) / 1e9;
        total_time += elapsed;
        
        double pi = 4.0 * (double)count / (double)NUM_SAMPLES;
        printf("Iteração %d: pi = %f, tempo = %f segundos\n", iter+1, pi, elapsed);
    }
    
    printf("Tempo médio: %f segundos\n", total_time / NUM_ITER);
    return 0;
}