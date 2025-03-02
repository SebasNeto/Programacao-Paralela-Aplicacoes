#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ITER 10

// Função de prefix sum sequencial (inclusivo)
void prefix_sum_seq(int *in, int *out, size_t n) {
    if (n == 0) return;
    out[0] = in[0];
    for (size_t i = 1; i < n; i++) {
        out[i] = out[i - 1] + in[i];
    }
}

int main(void) {
    size_t sizes[] = {10000000, 20000000, 30000000, 40000000, 50000000,
                      60000000, 70000000, 80000000, 90000000, 100000000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    double global_time_sum = 0.0;

    // Opcional: semente rand para consistência
    srand((unsigned) time(NULL));

    for (int s = 0; s < num_sizes; s++) {
        size_t n = sizes[s];
        int *in = malloc(n * sizeof(int));
        int *out = malloc(n * sizeof(int));
        if (!in || !out) {
            fprintf(stderr, "Erro na alocação de memória para tamanho %zu\n", n);
            exit(EXIT_FAILURE);
        }
        // Preenche o vetor de entrada com valores aleatórios entre 0 e 9
        for (size_t i = 0; i < n; i++) {
            in[i] = rand() % 10;
        }

        double time_sum = 0.0;
        for (int iter = 0; iter < NUM_ITER; iter++) {
            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);
            prefix_sum_seq(in, out, n);
            clock_gettime(CLOCK_MONOTONIC, &end);
            double elapsed = (end.tv_sec - start.tv_sec) +
                             (end.tv_nsec - start.tv_nsec) / 1e9;
            time_sum += elapsed;
        }
        double avg_time = time_sum / NUM_ITER;
        global_time_sum += avg_time;
        printf("Tamanho do vetor: %zu -> Tempo médio: %f segundos\n", n, avg_time);
        free(in);
        free(out);
    }
    double global_avg = global_time_sum / num_sizes;
    printf("\nTempo médio global: %f segundos\n", global_avg);
    return 0;
}
