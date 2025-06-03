import matplotlib.pyplot as plt
import networkx as nx
import time

# Layout hierárquico da árvore
def hierarchy_pos(G, root, width=1.0, vert_gap=0.3, vert_loc=0, xcenter=0.5):
    pos = {}
    def _hierarchy(G, root, left, width, vert_gap, vert_loc, xcenter, pos):
        pos[root] = (xcenter, vert_loc)
        filhos = list(G.successors(root))1
        if filhos:
            dx = width / len(filhos)
            nextx = xcenter - width/2 - dx/2
            for filho in filhos:
                nextx += dx
                pos = _hierarchy(G, filho, left, dx, vert_gap, vert_loc - vert_gap, nextx, pos)
        return pos
    return _hierarchy(G, root, 0, width, vert_gap, vert_loc, xcenter, pos)

# Define cor por busca
def cor_por_id(busca_id):
    cores = ['red', 'blue', 'green', 'orange', 'purple', 'brown', 'pink', 'cyan']
    return cores[busca_id % len(cores)]

# Busca binária paralela visual sobre grafo manual
def busca_binaria_multipla(array, alvos, G, pos, raiz):
    n_buscas = len(alvos)
    estados = [{'esq': 0, 'dir': len(array)-1, 'encontrado': False, 'meio': None, 'caminho': []}
               for _ in range(n_buscas)]
    
    max_passos = int.bit_length(len(array))
    for passo in range(max_passos + 1):
        plt.clf()
        cores = {}

        for i, alvo in enumerate(alvos):
            estado = estados[i]
            if estado['encontrado'] or estado['esq'] > estado['dir']:
                continue

            esq, dir = estado['esq'], estado['dir']
            meio = (esq + dir) // 2
            atual = array[meio]
            estado['meio'] = atual
            estado['caminho'].append(atual)

            if atual == alvo:
                estado['encontrado'] = True
            elif atual < alvo:
                estado['esq'] = meio + 1
            else:
                estado['dir'] = meio - 1

        # Define cores
        for i, estado in enumerate(estados):
            cor = cor_por_id(i)
            for no in estado['caminho']:
                if no not in cores:
                    cores[no] = 'gray'
            if estado['meio'] is not None:
                if estado['encontrado']:
                    cores[estado['meio']] = 'lime'
                else:
                    cores[estado['meio']] = cor

        node_colors = [cores.get(n, 'white') for n in G.nodes]
        nx.draw(G, pos, with_labels=True, node_color=node_colors,
                node_size=700, edge_color="lightgray", arrows=False)
        plt.title("Buscas Binárias Paralelas")
        plt.pause(1)

    plt.ioff()
    plt.show()

# ==========================================
# === MONTE SEU PRÓPRIO GRAFO AQUI 👇 =======
# ==========================================

# Exemplo: árvore binária manual
array = [0, 2, 4, 6, 8, 10, 12, 14, 16]
alvos = [6, 14, 12, 100]

G = nx.DiGraph()
# Adiciona nós (com os mesmos valores do array)
for val in array:
    G.add_node(val)

# Adiciona arestas manualmente (como uma árvore binária)
G.add_edge(8, 4)
G.add_edge(8, 12)
G.add_edge(4, 2)
G.add_edge(4, 6)
G.add_edge(2, 0)
G.add_edge(12, 10)
G.add_edge(12, 14)
G.add_edge(14, 16)

# Layout fixo baseado na raiz (8)
pos = hierarchy_pos(G, root=8)

# ==========================================
# ==== EXECUTA VISUALIZAÇÃO ================
# ==========================================

print("Alvos:", alvos)
busca_binaria_multipla(array, alvos, G, pos, raiz=8)
