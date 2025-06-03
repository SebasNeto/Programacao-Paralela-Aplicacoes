import matplotlib.pyplot as plt
import networkx as nx
import threading
import time
import random
from collections import deque

def criar_grafo_aleatorio(n, grau_medio):
    G = nx.Graph()
    G.add_nodes_from(range(n))

    # Conecta todos os vértices com uma árvore básica
    for i in range(1, n):
        pai = random.randint(0, i - 1)
        G.add_edge(i, pai)

    total_arestas = (grau_medio * n) // 2
    while G.number_of_edges() < total_arestas:
        u, v = random.randint(0, n - 1), random.randint(0, n - 1)
        if u != v and not G.has_edge(u, v):
            G.add_edge(u, v)

    return G

def visualizar_grafo(G, visitados, fronteira, pos, nivel):
    plt.clf()
    cores = []
    for node in G.nodes:
        if node in fronteira:
            cores.append("red")
        elif visitados[node]:
            cores.append("gray")
        else:
            cores.append("white")

    nx.draw(G, pos, with_labels=True, node_color=cores, node_size=300, edge_color="lightgray")
    plt.title(f"BFS — Camada {nivel}")
    plt.pause(0.8)

def bfs_visual_paralelo(G, inicio):
    n = G.number_of_nodes()
    visitados = [False] * n
    distancias = [-1] * n
    fronteira = deque([inicio])
    visitados[inicio] = True
    distancias[inicio] = 0

    pos = nx.spring_layout(G, seed=42)
    nivel = 0

    while fronteira:
        atual_fronteira = list(fronteira)
        fronteira.clear()
        threads = []

        print(f"Camada {nivel}: {atual_fronteira}")
        visualizar_grafo(G, visitados, atual_fronteira, pos, nivel)

        def processar(u):
            for vizinho in G.neighbors(u):
                if not visitados[vizinho]:
                    visitados[vizinho] = True
                    distancias[vizinho] = nivel + 1
                    fronteira.append(vizinho)

        for u in atual_fronteira:
            t = threading.Thread(target=processar, args=(u,))
            threads.append(t)
            t.start()

        for t in threads:
            t.join()

        nivel += 1

    plt.title("BFS Finalizada")
    plt.pause(2)
    plt.ioff()
    plt.show()

# Executa com 50 nós
plt.ion()
G = criar_grafo_aleatorio(100, grau_medio=3)
bfs_visual_paralelo(G, 0)

