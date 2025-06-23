#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ITERACOES 1

// Lista de tamanhos de amostras
long long tamanhos_amostras[] = {
    10000000, 20000000, 30000000, 40000000, 50000000,
    60000000, 70000000, 80000000, 90000000, 100000000
};

#define NUM_TAMANHOS (sizeof(tamanhos_amostras) / sizeof(tamanhos_amostras[0]))

double numero_aleatorio() {
    return (double)rand() / (double)RAND_MAX;
}

int main(void) {
    srand(time(NULL));
    
    for (int t = 0; t < NUM_TAMANHOS; t++) {
        long long NUM_AMOSTRAS = tamanhos_amostras[t];
        double tempo_total = 0.0;
        
        printf("\nTamanho da amostra: %lld\n", NUM_AMOSTRAS);
        
        for (int iteracao = 0; iteracao < NUM_ITERACOES; iteracao++) {
            long long contador = 0;
            struct timespec tempo_inicio, tempo_fim;
            
            clock_gettime(CLOCK_MONOTONIC, &tempo_inicio);
            
            // Geração das amostras e contagem dos pontos dentro do círculo
            for (long long i = 0; i < NUM_AMOSTRAS; i++) {
                double x = numero_aleatorio() * 2.0 - 1.0;
                double y = numero_aleatorio() * 2.0 - 1.0;
                if (x*x + y*y <= 1.0)
                    contador++;
            }
            
            clock_gettime(CLOCK_MONOTONIC, &tempo_fim);
            double tempo_decorrido = (tempo_fim.tv_sec - tempo_inicio.tv_sec) +
                                     (tempo_fim.tv_nsec - tempo_inicio.tv_nsec) / 1e9;
            tempo_total += tempo_decorrido;
            
            double pi_estimado = 4.0 * (double)contador / (double)NUM_AMOSTRAS;
            printf("Iteração %d: pi = %f, tempo = %f segundos\n", iteracao + 1, pi_estimado, tempo_decorrido);
        }
        
        printf("Tempo médio para amostra %lld: %f segundos\n", NUM_AMOSTRAS, tempo_total / NUM_ITERACOES);
    }
    return 0;
}


