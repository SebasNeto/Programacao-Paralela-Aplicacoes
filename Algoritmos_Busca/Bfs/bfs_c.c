#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define GRAU_MEDIO 10  // Grau médio desejado para o grafo

// Lista de tamanhos de vértices
long tamanhos[] = {1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 7000000, 8000000, 9000000};
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

// Adiciona uma aresta (para grafo não direcionado, chame para ambos os nós)
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

// Cria um grafo aleatório conectado com num_vertices e grau médio grau_medio
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
    // Adiciona arestas extras para atingir o grau médio 
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

//-------------------- FILA PARA BFS --------------------//
typedef struct {
    int *dados;
    long frente;
    long tras;
    long capacidade;
} Fila;

Fila *criar_fila(long capacidade) {
    Fila *f = malloc(sizeof(Fila));
    f->dados = malloc(capacidade * sizeof(int));
    f->frente = 0;
    f->tras = 0;
    f->capacidade = capacidade;
    return f;
}

void liberar_fila(Fila *f) {
    free(f->dados);
    free(f);
}

int fila_vazia(Fila *f) {
    return (f->frente == f->tras);
}

void enfileirar(Fila *f, int valor) {
    if (f->tras == f->capacidade) {
        f->capacidade *= 2;
        f->dados = realloc(f->dados, f->capacidade * sizeof(int));
        if (!f->dados) {
            perror("Erro no realloc da fila");
            exit(EXIT_FAILURE);
        }
    }
    f->dados[f->tras++] = valor;
}

int desenfileirar(Fila *f) {
    return f->dados[f->frente++];
}

//-------------------- BFS SEQUENCIAL --------------------//
void bfs_sequencial(Grafo *grafo, int inicio, int *distancia) {
    long n = grafo->num_vertices;
    for (long i = 0; i < n; i++)
        distancia[i] = -1;
    
    Fila *f = criar_fila(n);
    distancia[inicio] = 0;
    enfileirar(f, inicio);
    
    while (!fila_vazia(f)) {
        int u = desenfileirar(f);
        int d = distancia[u];
        No *no = &grafo->nos[u];
        for (int i = 0; i < no->num_vizinhos; i++) {
            int v = no->vizinhos[i];
            if (distancia[v] == -1) {
                distancia[v] = d + 1;
                enfileirar(f, v);
            }
        }
    }
    liberar_fila(f);
}

double diferenca_tempo(struct timespec inicio, struct timespec fim) {
    return (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
}

int main() {
    srand(time(NULL));
    
    double soma_tempos = 0.0;
    
    // Executa uma vez para cada tamanho do grafo
    for (int s = 0; s < num_tamanhos; s++) {
        long num_vertices = tamanhos[s];
        printf("Tamanho do grafo: %ld vértices\n", num_vertices);
        
        Grafo *grafo = criar_grafo(num_vertices, GRAU_MEDIO);
        int *distancia = malloc(num_vertices * sizeof(int));
        if (!distancia) {
            perror("Erro ao alocar vetor de distância");
            exit(EXIT_FAILURE);
        }
        
        struct timespec inicio, fim;
        clock_gettime(CLOCK_MONOTONIC, &inicio);
        bfs_sequencial(grafo, 0, distancia);
        clock_gettime(CLOCK_MONOTONIC, &fim);
        double tempo = diferenca_tempo(inicio, fim);
        soma_tempos += tempo;
        printf("  Tempo: %f segundos\n\n", tempo);
        
        free(distancia);
        liberar_grafo(grafo);
    }
    
    double media_geral = soma_tempos / num_tamanhos;
    printf("Tempo médio geral (Sequencial): %f segundos\n", media_geral);
    
    return 0;
}



