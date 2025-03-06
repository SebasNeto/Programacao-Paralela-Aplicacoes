#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define GRAU_MEDIO 10

long tamanhos[] = {500000, 600000, 700000, 800000, 900000, 1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 7000000};
int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);

// Estrutura de cada nó (lista de adjacência)
typedef struct {
    int num_vizinhos;
    int capacidade;
    int *vizinhos;
} No;

// Estrutura do grafo
typedef struct {
    long num_vertices;
    No *nos;
} Grafo;

// Função para adicionar aresta (grafo não direcionado)
void adicionar_aresta(Grafo *grafo, long u, long v) {
    No *no = &grafo->nos[u];
    if (no->num_vizinhos == no->capacidade) {
        no->capacidade = (no->capacidade == 0) ? 4 : no->capacidade * 2;
        no->vizinhos = realloc(no->vizinhos, no->capacidade * sizeof(int));
        if (!no->vizinhos) {
            perror("Erro no realloc");
            exit(EXIT_FAILURE);
        }
    }
    no->vizinhos[no->num_vizinhos++] = v;
}

// Cria grafo aleatório conectado com num_vertices e grau médio grau_medio
Grafo *criar_grafo(long num_vertices, int grau_medio) {
    Grafo *grafo = malloc(sizeof(Grafo));
    grafo->num_vertices = num_vertices;
    grafo->nos = malloc(num_vertices * sizeof(No));
    if (!grafo->nos) {
        perror("Erro ao alocar nós");
        exit(EXIT_FAILURE);
    }
    for (long i = 0; i < num_vertices; i++) {
        grafo->nos[i].num_vizinhos = 0;
        grafo->nos[i].capacidade = 0;
        grafo->nos[i].vizinhos = NULL;
    }
    // Cria árvore geradora para garantir conectividade
    for (long i = 1; i < num_vertices; i++) {
        long pai = rand() % i;
        adicionar_aresta(grafo, i, pai);
        adicionar_aresta(grafo, pai, i);
    }
    // Adiciona arestas extras para atingir o grau médio desejado
    long total_arestas = (grau_medio * num_vertices) / 2;
    long arestas_extras = total_arestas - (num_vertices - 1);
    for (long i = 0; i < arestas_extras; i++) {
        long u = rand() % num_vertices;
        long v = rand() % num_vertices;
        if (u == v) { i--; continue; }
        adicionar_aresta(grafo, u, v);
        adicionar_aresta(grafo, v, u);
    }
    return grafo;
}

void liberar_grafo(Grafo *grafo) {
    for (long i = 0; i < grafo->num_vertices; i++) {
        free(grafo->nos[i].vizinhos);
    }
    free(grafo->nos);
    free(grafo);
}

//-------------------- VARIÁVEIS GLOBAIS PARA BFS PARALELO --------------------//
Grafo *grafo_global;
int *distancia_global;
int *fronteira_global;
int *proxima_fronteira_global;
volatile int tamanho_fronteira_global;
volatile int contador_proxima_fronteira;
volatile int nivel_global;
int num_threads;

pthread_barrier_t barreira;

//-------------------- FUNÇÃO EXECUTADA POR CADA THREAD --------------------//
void *bfs_thread(void *arg) {
    int id = *(int *)arg;
    while (tamanho_fronteira_global > 0) {
        for (int i = id; i < tamanho_fronteira_global; i += num_threads) {
            int u = fronteira_global[i];
            int d = nivel_global;
            No *no = &grafo_global->nos[u];
            for (int j = 0; j < no->num_vizinhos; j++) {
                int v = no->vizinhos[j];
                if (distancia_global[v] == -1) {
                    if (__sync_bool_compare_and_swap(&(distancia_global[v]), -1, d + 1)) {
                        int indice = __sync_fetch_and_add(&contador_proxima_fronteira, 1);
                        proxima_fronteira_global[indice] = v;
                    }
                }
            }
        }
        pthread_barrier_wait(&barreira);
        if (id == 0) {
            tamanho_fronteira_global = contador_proxima_fronteira;
            int *temp = fronteira_global;
            fronteira_global = proxima_fronteira_global;
            proxima_fronteira_global = temp;
            contador_proxima_fronteira = 0;
            nivel_global++;
        }
        pthread_barrier_wait(&barreira);
    }
    return NULL;
}

//-------------------- FUNÇÃO PARA CALCULAR A DIFERENÇA DE TEMPO --------------------//
double diferenca_tempo(struct timespec inicio, struct timespec fim) {
    return (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
}

//-------------------- FUNÇÃO BFS PARALELO MELHORADA --------------------//
void bfs_paralelo_melhorado(Grafo *grafo, int inicio) {
    long n = grafo->num_vertices;
    for (long i = 0; i < n; i++)
        distancia_global[i] = -1;
    distancia_global[inicio] = 0;

    fronteira_global[0] = inicio;
    tamanho_fronteira_global = 1;
    contador_proxima_fronteira = 0;
    nivel_global = 0;

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    int *ids_threads = malloc(num_threads * sizeof(int));
    for (int t = 0; t < num_threads; t++) {
        ids_threads[t] = t;
        pthread_create(&threads[t], NULL, bfs_thread, &ids_threads[t]);
    }
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
    free(threads);
    free(ids_threads);
}

int main() {
    srand(time(NULL));

    double soma_tempos = 0.0;
    for (int s = 0; s < num_tamanhos; s++) {
        long num_vertices = tamanhos[s];
        printf("Tamanho do grafo: %ld vértices\n", num_vertices);
        Grafo *grafo = criar_grafo(num_vertices, GRAU_MEDIO);

        grafo_global = grafo;
        distancia_global = malloc(num_vertices * sizeof(int));
        fronteira_global = malloc(num_vertices * sizeof(int));
        proxima_fronteira_global = malloc(num_vertices * sizeof(int));

        num_threads = sysconf(_SC_NPROCESSORS_ONLN);
        if (num_threads < 1) num_threads = 1;
        pthread_barrier_init(&barreira, NULL, num_threads);

        struct timespec tempo_inicio, tempo_fim;
        clock_gettime(CLOCK_MONOTONIC, &tempo_inicio);
        bfs_paralelo_melhorado(grafo, 0);
        clock_gettime(CLOCK_MONOTONIC, &tempo_fim);

        double tempo = diferenca_tempo(tempo_inicio, tempo_fim);
        soma_tempos += tempo;
        printf("  Tempo: %f segundos\n\n", tempo);

        free(distancia_global);
        free(fronteira_global);
        free(proxima_fronteira_global);
        pthread_barrier_destroy(&barreira);
        liberar_grafo(grafo);
    }

    double media_geral = soma_tempos / num_tamanhos;
    printf("Tempo médio geral (BFS Paralelo Melhorado): %f segundos\n", media_geral);

    return 0;
}
