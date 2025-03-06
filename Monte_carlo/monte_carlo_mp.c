#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define NUM_ITERACOES 10
#define NUM_AMOSTRAS 100000000  

int main(void) {
    double tempo_total = 0.0;
    
    srand48(time(NULL));
    
    for (int iteracao = 0; iteracao < NUM_ITERACOES; iteracao++) {
        long long contador = 0;
        double tempo_inicio = omp_get_wtime();
        
        #pragma omp parallel reduction(+:contador)
        {
            unsigned int semente = omp_get_thread_num() + time(NULL);
            #pragma omp for
            for (long long i = 0; i < NUM_AMOSTRAS; i++) {
                double x = ((double)rand_r(&semente) / RAND_MAX) * 2.0 - 1.0;
                double y = ((double)rand_r(&semente) / RAND_MAX) * 2.0 - 1.0;
                if (x*x + y*y <= 1.0)
                    contador++;
            }
        }
        
        double tempo_fim = omp_get_wtime();
        double tempo_decorrido = tempo_fim - tempo_inicio;
        tempo_total += tempo_decorrido;
        
        double pi = 4.0 * (double)contador / (double)NUM_AMOSTRAS;
        printf("Iteração %d: pi = %f, tempo = %f segundos\n", iteracao + 1, pi, tempo_decorrido);
    }
    
    printf("Tempo médio: %f segundos\n", tempo_total / NUM_ITERACOES);
    return 0;
}
