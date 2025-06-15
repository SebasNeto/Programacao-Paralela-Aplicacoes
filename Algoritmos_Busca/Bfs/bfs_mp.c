#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>

#define GRAU_MEDIO 10
// Lista de tamanhos de vértices para teste (ajuste conforme necessário)
long tamanhos[] = {1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 700000, 8000000, 9000000};
int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);

// Estrutura que representa cada nó (lista de adjacência)
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
            perror("Erro no realloc de vizinhos");
            exit(EXIT_FAILURE);
        }
    }
    no->vizinhos[no->num_vizinhos++] = v;
}

// Cria um grafo aleatório conectado com num_vertices e grau médio grau_medio
Grafo *criar_grafo(long num_vertices, int grau_medio) {
    Grafo *grafo = malloc(sizeof(Grafo));
    grafo->num_vertices = num_vertices;
    grafo->nos = malloc(num_vertices * sizeof(No));
    if (!grafo->nos) {
        perror("Erro ao alocar nós do grafo");
        exit(EXIT_FAILURE);
    }
    for (long i = 0; i < num_vertices; i++) {
        grafo->nos[i].num_vizinhos = 0;
        grafo->nos[i].capacidade = 0;
        grafo->nos[i].vizinhos = NULL;
    }
    // Cria uma árvore geradora para garantir conectividade
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

// Implementação da BFS utilizando OpenMP
void bfs_openmp(Grafo *grafo, int inicio, int *distancia) {
    long n = grafo->num_vertices;
    // Inicializa as distâncias como -1 (não visitado)
    for (long i = 0; i < n; i++)
        distancia[i] = -1;
    distancia[inicio] = 0;
    
    // Aloca vetores para a fronteira atual e para a próxima
    int *fronteira = malloc(n * sizeof(int));
    int *proxima_fronteira = malloc(n * sizeof(int));
    if (!fronteira || !proxima_fronteira) {
        perror("Erro ao alocar fronteiras");
        exit(EXIT_FAILURE);
    }
    int tamanho_fronteira = 0;
    fronteira[tamanho_fronteira++] = inicio;
    int nivel = 0;
    
    while (tamanho_fronteira > 0) {
        int contagem_proxima_fronteira = 0;
        // Processa os nós da fronteira atual em paralelo
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < tamanho_fronteira; i++) {
            int u = fronteira[i];
            No *no = &grafo->nos[u];
            for (int j = 0; j < no->num_vizinhos; j++) {
                int v = no->vizinhos[j];
                if (distancia[v] == -1) {
                    // Tenta marcar o vértice como visitado de forma atômica
                    if (__sync_bool_compare_and_swap(&distancia[v], -1, nivel + 1)) {
                        int pos = __sync_fetch_and_add(&contagem_proxima_fronteira, 1);
                        proxima_fronteira[pos] = v;
                    }
                }
            }
        }
        tamanho_fronteira = contagem_proxima_fronteira;
        // Troca os vetores para o próximo nível
        int *temp = fronteira;
        fronteira = proxima_fronteira;
        proxima_fronteira = temp;
        nivel++;
    }
    free(fronteira);
    free(proxima_fronteira);
}

// Função para calcular a diferença de tempo (em segundos)
double diferenca_tempo(struct timespec inicio, struct timespec fim) {
    return (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
}

int main() {
    srand(time(NULL));
    double soma_tempos = 0.0;
    
    for (int s = 0; s < num_tamanhos; s++) {
        long num_vertices = tamanhos[s];
        printf("Tamanho do grafo: %ld vértices\n", num_vertices);
        Grafo *grafo = criar_grafo(num_vertices, GRAU_MEDIO);
        int *distancia = malloc(num_vertices * sizeof(int));
        if (!distancia) {
            perror("Erro ao alocar vetor de distância");
            exit(EXIT_FAILURE);
        }
        
        struct timespec tempo_inicio, tempo_fim;
        clock_gettime(CLOCK_MONOTONIC, &tempo_inicio);
        bfs_openmp(grafo, 0, distancia);
        clock_gettime(CLOCK_MONOTONIC, &tempo_fim);
        
        double tempo = diferenca_tempo(tempo_inicio, tempo_fim);
        soma_tempos += tempo;
        printf("  Tempo: %f segundos\n\n", tempo);
        
        free(distancia);
        liberar_grafo(grafo);
    }
    
    double media_geral = soma_tempos / num_tamanhos;
    printf("Tempo médio (BFS com OpenMP): %f segundos\n", media_geral);
    
    return 0;
}



#pragma omp parallel for schedule(dynamic)
for (int i = 0; i < tam_fronteira; i++) {
    int u = fronteira[i];
    for (int v : adj[u])
        if (dist[v] == -1 &&
            __sync_bool_compare_and_swap(&dist[v], -1, dist[u] + 1))
            proxima[ __sync_fetch_and_add(&tam_prox, 1) ] = v;
}



