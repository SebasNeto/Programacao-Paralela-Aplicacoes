#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ITER 10
#define NUM_SAMPLES 100000000  // Número de amostras para cada iteração

// Função auxiliar para gerar números aleatórios em [0, 1]
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
