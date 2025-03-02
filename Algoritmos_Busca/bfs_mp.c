#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>

#define AVG_DEGREE 10
// Lista de tamanhos de vértices para teste (ajuste conforme necessário)
long sizes[] = {500000, 600000, 700000, 800000, 900000, 1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 7000000};
int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
//correct
// Estrutura que representa cada nó (lista de adjacência)
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
            perror("Erro no realloc de neighbors");
            exit(EXIT_FAILURE);
        }
    }
    node->neighbors[node->num_neighbors++] = v;
}

// Cria um grafo aleatório conectado com num_vertices e grau médio avg_degree
Graph *create_graph(long num_vertices, int avg_degree) {
    Graph *graph = malloc(sizeof(Graph));
    graph->num_vertices = num_vertices;
    graph->nodes = malloc(num_vertices * sizeof(Node));
    if (!graph->nodes) {
        perror("Erro ao alocar nodes do grafo");
        exit(EXIT_FAILURE);
    }
    for (long i = 0; i < num_vertices; i++) {
        graph->nodes[i].num_neighbors = 0;
        graph->nodes[i].capacity = 0;
        graph->nodes[i].neighbors = NULL;
    }
    // Cria uma árvore geradora para garantir conectividade
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

// Implementação da BFS utilizando OpenMP
void openmp_bfs(Graph *graph, int start, int *distance) {
    long n = graph->num_vertices;
    // Inicializa as distâncias como -1 (não visitado)
    for (long i = 0; i < n; i++)
        distance[i] = -1;
    distance[start] = 0;
    
    // Aloca vetores para a fronteira atual e para a próxima
    int *frontier = malloc(n * sizeof(int));
    int *next_frontier = malloc(n * sizeof(int));
    if (!frontier || !next_frontier) {
        perror("Erro ao alocar fronteiras");
        exit(EXIT_FAILURE);
    }
    int frontier_size = 0;
    frontier[frontier_size++] = start;
    int level = 0;
    
    while (frontier_size > 0) {
        int next_frontier_count = 0;
        // Processa os nós da fronteira atual em paralelo
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < frontier_size; i++) {
            int u = frontier[i];
            Node *node = &graph->nodes[u];
            for (int j = 0; j < node->num_neighbors; j++) {
                int v = node->neighbors[j];
                if (distance[v] == -1) {
                    // Tenta marcar o vértice como visitado de forma atômica
                    if (__sync_bool_compare_and_swap(&distance[v], -1, level + 1)) {
                        int pos = __sync_fetch_and_add(&next_frontier_count, 1);
                        next_frontier[pos] = v;
                    }
                }
            }
        }
        frontier_size = next_frontier_count;
        // Troca os vetores para o próximo nível
        int *temp = frontier;
        frontier = next_frontier;
        next_frontier = temp;
        level++;
    }
    free(frontier);
    free(next_frontier);
}

// Função para calcular a diferença de tempo (em segundos)
double time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main() {
    srand(time(NULL));
    double total_time = 0.0;
    
    for (int s = 0; s < num_sizes; s++) {
        long num_vertices = sizes[s];
        printf("Tamanho do grafo: %ld vértices\n", num_vertices);
        Graph *graph = create_graph(num_vertices, AVG_DEGREE);
        int *distance = malloc(num_vertices * sizeof(int));
        if (!distance) {
            perror("Erro ao alocar vetor distance");
            exit(EXIT_FAILURE);
        }
        
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        openmp_bfs(graph, 0, distance);
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        
        double t = time_diff(start_time, end_time);
        total_time += t;
        printf("  Tempo: %f segundos\n\n", t);
        
        free(distance);
        free_graph(graph);
    }
    
    double average_time = total_time / num_sizes;
    printf("Tempo médio (BFS com OpenMP): %f segundos\n", average_time);
    
    return 0;
}
