#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define NUM_BUSCAS 1000

// Função de busca binária (array de inteiros ordenados)
int binary_search(const int *arr, int n, int x) {
    int left = 0, right = n - 1;
    while (left <= right) {
        int mid = (left + right) >> 1;  // divisão por 2 usando bitwise shift
        if (arr[mid] == x)
            return mid;
        else if (arr[mid] < x)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return -1;
}

// Estrutura para passar os dados de cada thread
typedef struct {
    const int *arr;      // Array ordenado
    int n;               // Tamanho do array
    int *buscas;         // Array de valores a buscar
    int *resultados;     // Array para armazenar os resultados
    int start;           // Índice inicial para as buscas atribuídas à thread
    int end;             // Índice final (não incluso) para as buscas
} thread_data;

// Função executada por cada thread
void *thread_func(void *arg) {
    thread_data *data = (thread_data *) arg;
    for (int i = data->start; i < data->end; i++) {
        data->resultados[i] = binary_search(data->arr, data->n, data->buscas[i]);
    }
    return NULL;
}

int main() {
    // Lista de tamanhos para os arrays (atenção: tamanhos muito grandes podem demandar muita memória)
    int tamanhos[] = {1000000, 5000000, 10000000, 25000000, 50000000, 75000000, 100000000, 250000000, 500000000};
    int num_sizes = sizeof(tamanhos) / sizeof(tamanhos[0]);
    double soma_tempos = 0.0;

    // Semente para o gerador de números aleatórios
    srand(time(NULL));

    // Obtém o número de threads disponíveis (baseado nos núcleos do sistema)
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if(num_threads < 1) num_threads = 1;
    printf("Usando %d threads.\n", num_threads);

    for (int s = 0; s < num_sizes; s++) {
        int n = tamanhos[s];

        // Aloca e inicializa o array ordenado com números ímpares: 1, 3, 5, ..., (2*n - 1)
        int *arr = malloc(n * sizeof(int));
        if(arr == NULL) {
            fprintf(stderr, "Erro ao alocar memória para o array de tamanho %d\n", n);
            exit(1);
        }
        for (int i = 0; i < n; i++) {
            arr[i] = 1 + 2 * i;
        }

        // Cria o array de buscas com números aleatórios entre 1 e 2*n
        int *buscas = malloc(NUM_BUSCAS * sizeof(int));
        if(buscas == NULL) {
            fprintf(stderr, "Erro ao alocar memória para o array de buscas\n");
            free(arr);
            exit(1);
        }
        for (int i = 0; i < NUM_BUSCAS; i++) {
            buscas[i] = (rand() % (2 * n)) + 1;
        }

        // Array para armazenar os resultados da busca
        int *resultados = malloc(NUM_BUSCAS * sizeof(int));
        if(resultados == NULL) {
            fprintf(stderr, "Erro ao alocar memória para o array de resultados\n");
            free(arr);
            free(buscas);
            exit(1);
        }

        // Cria as threads para executar as buscas em paralelo
        pthread_t threads[num_threads];
        thread_data tdata[num_threads];
        int chunk = (NUM_BUSCAS + num_threads - 1) / num_threads; // divisão do trabalho entre threads

        // Marca o tempo de início
        struct timespec t_start, t_end;
        clock_gettime(CLOCK_MONOTONIC, &t_start);

        for (int t = 0; t < num_threads; t++) {
            tdata[t].arr = arr;
            tdata[t].n = n;
            tdata[t].buscas = buscas;
            tdata[t].resultados = resultados;
            tdata[t].start = t * chunk;
            tdata[t].end = (t + 1) * chunk;
            if (tdata[t].end > NUM_BUSCAS)
                tdata[t].end = NUM_BUSCAS;
            pthread_create(&threads[t], NULL, thread_func, &tdata[t]);
        }

        // Aguarda todas as threads finalizarem
        for (int t = 0; t < num_threads; t++) {
            pthread_join(threads[t], NULL);
        }

        // Marca o tempo de término e calcula o tempo decorrido
        clock_gettime(CLOCK_MONOTONIC, &t_end);
        double tempo = (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_nsec - t_start.tv_nsec) / 1e9;
        soma_tempos += tempo;
        printf("Tamanho: %d - Tempo da busca binária paralela: %f segundos\n", n, tempo);

        // Libera a memória alocada
        free(arr);
        free(buscas);
        free(resultados);
    }

    double media = soma_tempos / num_sizes;
    printf("Média do tempo de execução paralela: %f segundos\n", media);

    return 0;
}
