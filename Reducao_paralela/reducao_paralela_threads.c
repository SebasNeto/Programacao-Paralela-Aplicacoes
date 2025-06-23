#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h> // para sysconf

#define NUM_ITERACOES 10  // Número de repetições para cada tamanho

// Estrutura para passar dados para cada thread
typedef struct {
    int * restrict vetor;
    size_t inicio;
    size_t fim;
    long long soma_parcial;
} DadosThread;

// Função que cada thread executará para calcular a soma parcial
void *funcao_thread(void *arg) {
    DadosThread *dados = (DadosThread*) arg;
    long long soma = 0;
    for (size_t i = dados->inicio; i < dados->fim; i++) {
        soma += dados->vetor[i];
    }
    dados->soma_parcial = soma;
    return NULL;
}

// Função para preencher o vetor com números aleatórios entre 0 e 9
void gerar_vetor_aleatorio(int * restrict vetor, size_t n) {
    for (size_t i = 0; i < n; i++) {
        vetor[i] = rand() % 10;
    }
}

int main() {
    // Lista dos tamanhos de vetor para teste (de 10M a 100M elementos)
    size_t tamanhos[] = {10000000, 20000000, 30000000, 40000000, 50000000,
                         60000000, 70000000, 80000000, 90000000, 100000000};
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);

    // Obtém o número de threads disponíveis no sistema
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads < 1)
        num_threads = 1;

    double tempo_total = 0.0;
    volatile long long soma_total_dummy = 0;  // Variável dummy para evitar otimização

    srand((unsigned) time(NULL));

    printf("Benchmark de Redução Paralela com POSIX Threads\n");
    printf("Número de iterações por tamanho: %d\n", NUM_ITERACOES);
    printf("Número de threads: %d\n\n", num_threads);

    for (int t = 0; t < num_tamanhos; t++) {
        size_t n = tamanhos[t];

        // Aloca memória alinhada a 64 bytes para melhor desempenho de cache e vetorização
        int * restrict vetor = NULL;
        if (posix_memalign((void**)&vetor, 64, n * sizeof(int)) != 0) {
            fprintf(stderr, "Erro na alocação de memória para tamanho %zu\n", n);
            return EXIT_FAILURE;
        }

        gerar_vetor_aleatorio(vetor, n);
        double tempo_medio = 0.0;

        // Executa NUM_ITERACOES iterações para cada tamanho
        for (int iteracao = 0; iteracao < NUM_ITERACOES; iteracao++) {
            struct timespec tempo_inicio, tempo_fim;
            clock_gettime(CLOCK_MONOTONIC, &tempo_inicio);

            // Cria arrays para threads e seus dados
            pthread_t threads[num_threads];
            DadosThread dados_threads[num_threads];

            // Divide o vetor entre as threads
            size_t bloco = n / num_threads;
            size_t resto = n % num_threads;
            size_t indice_inicio = 0;
            for (int i = 0; i < num_threads; i++) {
                dados_threads[i].vetor = vetor;
                dados_threads[i].inicio = indice_inicio;
                // Distribui o resto entre as primeiras threads
                size_t extra = (i < resto) ? 1 : 0;
                dados_threads[i].fim = indice_inicio + bloco + extra;
                dados_threads[i].soma_parcial = 0;
                indice_inicio = dados_threads[i].fim;
                if (pthread_create(&threads[i], NULL, funcao_thread, &dados_threads[i]) != 0) {
                    fprintf(stderr, "Erro ao criar thread %d\n", i);
                    free(vetor);
                    return EXIT_FAILURE;
                }
            }

            // Junta todas as threads e soma os resultados parciais
            long long soma_reducao = 0;
            for (int i = 0; i < num_threads; i++) {
                pthread_join(threads[i], NULL);
                soma_reducao += dados_threads[i].soma_parcial;
            }

            clock_gettime(CLOCK_MONOTONIC, &tempo_fim);
            double tempo_execucao = (tempo_fim.tv_sec - tempo_inicio.tv_sec) +
                                    (tempo_fim.tv_nsec - tempo_inicio.tv_nsec) / 1e9;
            tempo_medio += tempo_execucao;

            // Acumula a soma para evitar que o compilador otimize a operação
            soma_total_dummy += soma_reducao;
        }

        tempo_medio /= NUM_ITERACOES;
        tempo_total += tempo_medio;
        printf("Tamanho do vetor: %zu -> Tempo médio: %lf segundos\n", n, tempo_medio);

        free(vetor);
    }

    double tempo_medio_geral = tempo_total / num_tamanhos;
    printf("\nTempo médio geral: %lf segundos\n", tempo_medio_geral);
    printf("Soma total (dummy): %lld\n", soma_total_dummy);

    return 0;
}





