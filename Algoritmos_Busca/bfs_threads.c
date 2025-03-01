#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define AVG_DEGREE 10  // Grau médio desejado para o grafo
// Lista de tamanhos de vértices para teste (escolha tamanhos que permitam perceber ganhos sem gastar tempo demais)
long sizes[] = {1000000, 5000000, 10000000, 25000000};
int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

// Estrutura de cada nó (lista de adjacência)
typedef struct {
    int num_neighbors;
    int capacity;
    int *neighbors;
} Node;

// Estrutura do grafo
typedef struct {
    long num_vertices;
    Node *nodes;
} Graph;

// Função para adicionar aresta (grafo não direcionado)
void add_edge(Graph *graph, long u, long v) {
    Node *node = &graph->nodes[u];
    if (node->num_neighbors == node->capacity) {
        node->capacity = (node->capacity == 0) ? 4 : node->capacity * 2;
        node->neighbors = realloc(node->neighbors, node->capacity * sizeof(int));
        if (!node->neighbors) {
            perror("Erro no realloc");
            exit(EXIT_FAILURE);
        }
    }
    node->neighbors[node->num_neighbors++] = v;
}

// Cria grafo aleatório conectado com num_vertices e grau médio avg_degree
Graph *create_graph(long num_vertices, int avg_degree) {
    Graph *graph = malloc(sizeof(Graph));
    graph->num_vertices = num_vertices;
    graph->nodes = malloc(num_vertices * sizeof(Node));
    if (!graph->nodes) {
        perror("Erro ao alocar nodes");
        exit(EXIT_FAILURE);
    }
    for (long i = 0; i < num_vertices; i++) {
        graph->nodes[i].num_neighbors = 0;
        graph->nodes[i].capacity = 0;
        graph->nodes[i].neighbors = NULL;
    }
    // Cria árvore geradora para garantir conectividade
    for (long i = 1; i < num_vertices; i++) {
        long parent = rand() % i;
        add_edge(graph, i, parent);
        add_edge(graph, parent, i);
    }
    // Adiciona arestas extras para atingir o grau médio desejado
    long total_edges = (avg_degree * num_vertices) / 2;
    long extra_edges = total_edges - (num_vertices - 1);
    for (long i = 0; i < extra_edges; i++) {
        long u = rand() % num_vertices;
        long v = rand() % num_vertices;
        if (u == v) { i--; continue; }
        add_edge(graph, u, v);
        add_edge(graph, v, u);
    }
    return graph;
}

void free_graph(Graph *graph) {
    for (long i = 0; i < graph->num_vertices; i++) {
        free(graph->nodes[i].neighbors);
    }
    free(graph->nodes);
    free(graph);
}

//-------------------- VARIÁVEIS GLOBAIS PARA BFS PARARELO --------------------//
Graph *global_graph;
int *global_distance;
int *global_frontier;       // Vetor com os nós do nível atual
int *global_next_frontier;  // Vetor para construir o próximo nível
volatile int global_frontier_size;
volatile int next_frontier_counter;
volatile int global_level;
int num_threads;

pthread_barrier_t barrier;

//-------------------- FUNÇÃO EXECUTADA POR CADA THREAD --------------------//
void *bfs_thread(void *arg) {
    int tid = *(int *)arg;
    while (global_frontier_size > 0) {
        // Cada thread processa os índices com stride = num_threads
        for (int i = tid; i < global_frontier_size; i += num_threads) {
            int u = global_frontier[i];
            int d = global_level;
            Node *node = &global_graph->nodes[u];
            for (int j = 0; j < node->num_neighbors; j++) {
                int v = node->neighbors[j];
                // Se o nó não foi visitado, tenta marcar atômicamente
                if (global_distance[v] == -1) {
                    if (__sync_bool_compare_and_swap(&(global_distance[v]), -1, d + 1)) {
                        int index = __sync_fetch_and_add(&next_frontier_counter, 1);
                        global_next_frontier[index] = v;
                    }
                }
            }
        }
        // Espera todos as threads terminarem o processamento do nível atual
        pthread_barrier_wait(&barrier);

        // Apenas uma thread (por exemplo, tid 0) prepara o próximo nível
        if (tid == 0) {
            global_frontier_size = next_frontier_counter;
            // Troca os vetores de fronteira
            int *temp = global_frontier;
            global_frontier = global_next_frontier;
            global_next_frontier = temp;
            next_frontier_counter = 0;
            global_level++;
        }
        // Sincroniza para garantir que a troca já ocorreu
        pthread_barrier_wait(&barrier);
    }
    return NULL;
}

//-------------------- FUNÇÃO PARA CALCULAR A DIFERENÇA DE TEMPO --------------------//
double time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

//-------------------- FUNÇÃO BFS PARARELO MELHORADA --------------------//
void improved_parallel_bfs(Graph *graph, int start) {
    long n = graph->num_vertices;
    // Inicializa o vetor de distâncias
    for (long i = 0; i < n; i++)
        global_distance[i] = -1;
    global_distance[start] = 0;

    // Inicializa a fronteira com o nó de partida
    global_frontier[0] = start;
    global_frontier_size = 1;
    next_frontier_counter = 0;
    global_level = 0;

    // Cria as threads (elas serão criadas apenas uma vez)
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    int *thread_ids = malloc(num_threads * sizeof(int));
    for (int t = 0; t < num_threads; t++) {
        thread_ids[t] = t;
        pthread_create(&threads[t], NULL, bfs_thread, &thread_ids[t]);
    }
    // A thread principal aguarda as threads terminarem (quando o frontier acabar)
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
    free(threads);
    free(thread_ids);
}

int main() {
    srand(time(NULL));

    // Para cada tamanho de grafo, executa o BFS e acumula o tempo
    double soma_tempos = 0.0;
    for (int s = 0; s < num_sizes; s++) {
        long num_vertices = sizes[s];
        printf("Tamanho do grafo: %ld vértices\n", num_vertices);
        Graph *graph = create_graph(num_vertices, AVG_DEGREE);

        // Aloca memória para variáveis globais
        global_graph = graph;
        global_distance = malloc(num_vertices * sizeof(int));
        // Para as fronteiras, alocamos espaço para até num_vertices nós
        global_frontier = malloc(num_vertices * sizeof(int));
        global_next_frontier = malloc(num_vertices * sizeof(int));

        // Define o número de threads com base no número de processadores disponíveis
        num_threads = sysconf(_SC_NPROCESSORS_ONLN);
        if (num_threads < 1) num_threads = 1;
        // Inicializa a barreira para sincronização das threads
        pthread_barrier_init(&barrier, NULL, num_threads);

        struct timespec start_time, end_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        improved_parallel_bfs(graph, 0);
        clock_gettime(CLOCK_MONOTONIC, &end_time);

        double t = time_diff(start_time, end_time);
        soma_tempos += t;
        printf("  Tempo: %f segundos\n\n", t);

        // Libera memória
        free(global_distance);
        free(global_frontier);
        free(global_next_frontier);
        pthread_barrier_destroy(&barrier);
        free_graph(graph);
    }

    double media_geral = soma_tempos / num_sizes;
    printf("Tempo médio geral (Paralelo Melhorado): %f segundos\n", media_geral);

    return 0;
}
