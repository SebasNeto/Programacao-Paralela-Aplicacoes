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
