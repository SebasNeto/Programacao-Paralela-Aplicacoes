#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_ITERACOES 1
#define NUM_THREADS 16 // Número de threads a serem utilizadas

// Lista de tamanhos de amostras
long long tamanhos_amostras[] = {
    10000000, 20000000, 30000000, 40000000, 50000000,
    60000000, 70000000, 80000000, 90000000, 100000000
};

#define NUM_TAMANHOS (sizeof(tamanhos_amostras) / sizeof(tamanhos_amostras[0]))

// Estrutura para armazenar os dados passados para cada thread
typedef struct {
    long long amostras;  
    unsigned int semente; 
    long long contador; 
} dados_thread_t;

void* monteCarlo(void* arg) {
    dados_thread_t *dados = (dados_thread_t*) arg;
    long long contador_local = 0;
    for (long long i = 0; i < dados->amostras; i++) {
        // Gera números aleatórios no intervalo [-1, 1]
        double x = ((double)rand_r(&dados->semente) / RAND_MAX) * 2.0 - 1.0;
        double y = ((double)rand_r(&dados->semente) / RAND_MAX) * 2.0 - 1.0;
        if (x * x + y * y <= 1.0)
            contador_local++;
    }
    dados->contador = contador_local;
    pthread_exit(NULL);
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    dados_thread_t dados_threads[NUM_THREADS];

    // Semente global para diferenciar as sementes de cada thread
    unsigned int semente_global = (unsigned int) time(NULL);
    
    for (int t = 0; t < NUM_TAMANHOS; t++) {
        long long NUM_AMOSTRAS = tamanhos_amostras[t];
        double tempo_total = 0.0;
        
        printf("\nTamanho da amostra: %lld\n", NUM_AMOSTRAS);
        
        for (int iteracao = 0; iteracao < NUM_ITERACOES; iteracao++) {
            long long contador_total = 0;
            struct timespec tempo_inicio, tempo_fim;
            clock_gettime(CLOCK_MONOTONIC, &tempo_inicio);
            
            // Divide o número total de amostras entre as threads
            long long amostras_por_thread = NUM_AMOSTRAS / NUM_THREADS;
            long long resto = NUM_AMOSTRAS % NUM_THREADS;
            
            // Cria as threads
            for (int t = 0; t < NUM_THREADS; t++) {
                dados_threads[t].amostras = amostras_por_thread + (t < resto ? 1 : 0);
                // Cria sementes diferentes para cada thread
                dados_threads[t].semente = semente_global + t + iteracao;
                dados_threads[t].contador = 0;
                if (pthread_create(&threads[t], NULL, monteCarlo, &dados_threads[t]) != 0) {
                    perror("pthread_create");
                    exit(EXIT_FAILURE);
                }
            }
            
            // Aguarda o término de todas as threads e acumula os resultados
            for (int t = 0; t < NUM_THREADS; t++) {
                pthread_join(threads[t], NULL);
                contador_total += dados_threads[t].contador;
            }
            
            clock_gettime(CLOCK_MONOTONIC, &tempo_fim);
            double tempo_decorrido = (tempo_fim.tv_sec - tempo_inicio.tv_sec) + (tempo_fim.tv_nsec - tempo_inicio.tv_nsec) / 1e9;
            tempo_total += tempo_decorrido;
            
            double pi_estimado = 4.0 * (double)contador_total / (double)NUM_AMOSTRAS;
            printf("Iteração %d: pi = %f, tempo = %f segundos\n", iteracao + 1, pi_estimado, tempo_decorrido);
        }
        
        printf("Tempo médio para amostra %lld: %f segundos\n", NUM_AMOSTRAS, tempo_total / NUM_ITERACOES);
    }
    
    return 0;
}
