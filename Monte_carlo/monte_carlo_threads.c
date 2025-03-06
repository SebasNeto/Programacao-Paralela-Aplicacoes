#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_ITERACOES 10
#define NUM_AMOSTRAS 100000000LL  // Número total de amostras por iteração
#define NUM_THREADS 4             // Número de threads a serem utilizadas

// Estrutura para armazenar os dados passados para cada thread
typedef struct {
    long long amostras;  // Número de amostras que a thread deverá processar
    unsigned int semente; // Semente para o gerador rand_r
    long long contador;  // Contagem local dos pontos dentro do círculo
} dados_thread_t;

void* monte_carlo_thread(void* arg) {
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
    double tempo_total = 0.0;
    pthread_t threads[NUM_THREADS];
    dados_thread_t dados_threads[NUM_THREADS];

    // Semente global para diferenciar as sementes de cada thread
    unsigned int semente_global = (unsigned int) time(NULL);

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
            // Cria sementes diferentes para cada thread (exemplo: soma do ID com a semente global)
            dados_threads[t].semente = semente_global + t + iteracao;
            dados_threads[t].contador = 0;
            if (pthread_create(&threads[t], NULL, monte_carlo_thread, &dados_threads[t]) != 0) {
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
    
    printf("Tempo médio: %f segundos\n", tempo_total / NUM_ITERACOES);
    return 0;
}
