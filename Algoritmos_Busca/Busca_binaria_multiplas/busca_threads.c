#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define NUM_BUSCAS 1000

// Função de busca binária (array de inteiros ordenados)
int busca_binaria(const int *arr, int n, int x) {
    int esquerda = 0, direita = n - 1;
    while (esquerda <= direita) {
        int meio = (esquerda + direita) >> 1;  // divisão por 2 usando deslocamento de bits
        if (arr[meio] == x)
            return meio;
        else if (arr[meio] < x)
            esquerda = meio + 1;
            direita = meio - 1;
    }
    return -1;
}

// Estrutura para passar os dados de cada thread
typedef struct {
    const int *arr;      // Array ordenado
    int n;               // Tamanho do array
    int *buscas;         // Array de valores a buscar
    int *resultados;     // Array para armazenar os resultados
    int inicio;          // Índice inicial para as buscas atribuídas à thread
    int fim;             // Índice final (não incluso) para as buscas
} dados_thread;

// Função executada por cada thread
void *funcao_thread(void *arg) {
    dados_thread *dados = (dados_thread *) arg;
    for (int i = dados->inicio; i < dados->fim; i++) {
        dados->resultados[i] = busca_binaria(dados->arr, dados->n, dados->buscas[i]);
    }
    return NULL;
}

int main() {
    // Lista de tamanhos para os arrays (atenção: tamanhos muito grandes podem demandar muita memória)
    int tamanhos[] = {1000000, 5000000, 10000000, 25000000, 50000000, 75000000, 100000000, 250000000, 500000000};
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);
    double soma_tempos = 0.0;

    // Semente para o gerador de números aleatórios
    srand(time(NULL));

    // Obtém o número de threads disponíveis (baseado nos núcleos do sistema)
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if(num_threads < 1) num_threads = 1;
    printf("Usando %d threads.\n", num_threads);

    for (int s = 0; s < num_tamanhos; s++) {
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
        dados_thread dados_threads[num_threads];
        int tamanho_pedaco = (NUM_BUSCAS + num_threads - 1) / num_threads; // divisão do trabalho entre threads

        // Marca o tempo de início
        struct timespec inicio_tempo, fim_tempo;
        clock_gettime(CLOCK_MONOTONIC, &inicio_tempo);

        for (int t = 0; t < num_threads; t++) {
            dados_threads[t].arr = arr;
            dados_threads[t].n = n;
            dados_threads[t].buscas = buscas;
            dados_threads[t].resultados = resultados;
            dados_threads[t].inicio = t * tamanho_pedaco;
            dados_threads[t].fim = (t + 1) * tamanho_pedaco;
            if (dados_threads[t].fim > NUM_BUSCAS)
                dados_threads[t].fim = NUM_BUSCAS;
            pthread_create(&threads[t], NULL, funcao_thread, &dados_threads[t]);
        }

        // Aguarda todas as threads finalizarem
        for (int t = 0; t < num_threads; t++) {
            pthread_join(threads[t], NULL);
        }

        // Marca o tempo de término e calcula o tempo decorrido
        clock_gettime(CLOCK_MONOTONIC, &fim_tempo);
        double tempo = (fim_tempo.tv_sec - inicio_tempo.tv_sec) + (fim_tempo.tv_nsec - inicio_tempo.tv_nsec) / 1e9;
        soma_tempos += tempo;
        printf("Tamanho: %d - Tempo da busca binária paralela: %f segundos\n", n, tempo);

        // Libera a memória alocada
        free(arr);
        free(buscas);
        free(resultados);
    }

    double media = soma_tempos / num_tamanhos;
    printf("Média do tempo de execução paralela: %f segundos\n", media);

    return 0;
}
