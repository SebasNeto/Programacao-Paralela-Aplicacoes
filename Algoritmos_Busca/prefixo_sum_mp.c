#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include <time.h>
#define NUM_ITER 10

// Função de prefix sum paralela com OpenMP (inclusivo)
void prefix_sum_openmp(int *in, int *out, size_t n) {
    int num_threads;
    // Determina o número de threads
    #pragma omp parallel
    {
        #pragma omp single
        num_threads = omp_get_num_threads();
    }

    // Vetor para armazenar o último valor de cada chunk
    int *sums = malloc(num_threads * sizeof(int));
    if (!sums) {
        fprintf(stderr, "Erro na alocação de sums\n");
        exit(EXIT_FAILURE);
    }

    // Cada thread calcula o prefix sum do seu bloco
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        size_t chunk = n / num_threads;
        size_t start = tid * chunk;
        size_t end = (tid == num_threads - 1) ? n : start + chunk;

        if (start < end) {
            out[start] = in[start];
            for (size_t i = start + 1; i < end; i++) {
                out[i] = out[i - 1] + in[i];
            }
            sums[tid] = out[end - 1];
        } else {
            sums[tid] = 0;
        }
    }

    // Calcula os offsets: prefix sum dos últimos elementos de cada bloco
    for (int i = 1; i < num_threads; i++) {
        sums[i] += sums[i - 1];
    }

    // Ajusta os blocos (exceto o primeiro) adicionando o offset acumulado
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if (tid > 0) {
            size_t chunk = n / num_threads;
            size_t start = tid * chunk;
            size_t end = (tid == num_threads - 1) ? n : start + chunk;
            int offset = sums[tid - 1];
            for (size_t i = start; i < end; i++) {
                out[i] += offset;
            }
        }
    }
    free(sums);
}

int main(void) {
    size_t sizes[] = {10000000, 20000000, 30000000, 40000000, 50000000,
                      60000000, 70000000, 80000000, 90000000, 100000000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    double global_time_sum = 0.0;

    // Opcional: semente para rand
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
            double start_time = omp_get_wtime();
            prefix_sum_openmp(in, out, n);
            double end_time = omp_get_wtime();
            double elapsed = end_time - start_time;
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
