#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define AVG_DEGREE 10  // Grau médio desejado para o grafo

// Lista de tamanhos de vértices
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

// Adiciona uma aresta (para grafo não direcionado, chame para ambos os nós)
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

// Cria um grafo aleatório conectado com num_vertices e grau médio avg_degree
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

//-------------------- FILA PARA BFS --------------------//
typedef struct {
    int *data;
    long front;
    long rear;
    long capacity;
} Queue;

Queue *create_queue(long capacity) {
    Queue *q = malloc(sizeof(Queue));
    q->data = malloc(capacity * sizeof(int));
    q->front = 0;
    q->rear = 0;
    q->capacity = capacity;
    return q;
}

void free_queue(Queue *q) {
    free(q->data);
    free(q);
}

int is_empty(Queue *q) {
    return (q->front == q->rear);
}

void enqueue(Queue *q, int val) {
    if (q->rear == q->capacity) {
        q->capacity *= 2;
        q->data = realloc(q->data, q->capacity * sizeof(int));
        if (!q->data) {
            perror("Erro no realloc da fila");
            exit(EXIT_FAILURE);
        }
    }
    q->data[q->rear++] = val;
}

int dequeue(Queue *q) {
    return q->data[q->front++];
}

//-------------------- BFS SEQUENCIAL --------------------//
void sequential_bfs(Graph *graph, int start, int *distance) {
    long n = graph->num_vertices;
    for (long i = 0; i < n; i++)
        distance[i] = -1;
    
    Queue *q = create_queue(n);
    distance[start] = 0;
    enqueue(q, start);
    
    while (!is_empty(q)) {
        int u = dequeue(q);
        int d = distance[u];
        Node *node = &graph->nodes[u];
        for (int i = 0; i < node->num_neighbors; i++) {
            int v = node->neighbors[i];
            if (distance[v] == -1) {
                distance[v] = d + 1;
                enqueue(q, v);
            }
        }
    }
    free_queue(q);
}

// Função para calcular a diferença de tempo (em segundos)
double time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main() {
    srand(time(NULL));
    
    double soma_tempos = 0.0;
    
    // Executa uma vez para cada tamanho do grafo
    for (int s = 0; s < num_sizes; s++) {
        long num_vertices = sizes[s];
        printf("Tamanho do grafo: %ld vértices\n", num_vertices);
        
        Graph *graph = create_graph(num_vertices, AVG_DEGREE);
        int *distance = malloc(num_vertices * sizeof(int));
        if (!distance) {
            perror("Erro ao alocar vetor distance");
            exit(EXIT_FAILURE);
        }
        
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        sequential_bfs(graph, 0, distance);
        clock_gettime(CLOCK_MONOTONIC, &end);
        double t = time_diff(start, end);
        soma_tempos += t;
        printf("  Tempo: %f segundos\n\n", t);
        
        free(distance);
        free_graph(graph);
    }
    
    double media_geral = soma_tempos / num_sizes;
    printf("Tempo médio geral (Sequencial): %f segundos\n", media_geral);
    
    return 0;
}
