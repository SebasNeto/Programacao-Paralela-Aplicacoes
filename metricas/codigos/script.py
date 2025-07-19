# Reimportar bibliotecas e gerar novamente a imagem após o reset

import matplotlib.pyplot as plt
import networkx as nx

# Criar o grafo da árvore de tarefas
G = nx.DiGraph()

# Adicionando nós (representando as partições)
G.add_node("Vetor 10M")
G.add_node("5M Esq")
G.add_node("5M Dir")
G.add_node("2.5M Esq1")
G.add_node("2.5M Esq2")
G.add_node("2.5M Dir1")
G.add_node("2.5M Dir2")
G.add_node("1.25M Esq1")
G.add_node("1.25M Esq2")
G.add_node("1.25M Esq3")
G.add_node("1.25M Esq4")
G.add_node("1.25M Dir1")
G.add_node("1.25M Dir2")
G.add_node("1.25M Dir3")
G.add_node("1.25M Dir4")

# Adicionando arestas (representando chamadas recursivas)
G.add_edges_from([
    ("Vetor 10M", "5M Esq"), ("Vetor 10M", "5M Dir"),
    ("5M Esq", "2.5M Esq1"), ("5M Esq", "2.5M Esq2"),
    ("5M Dir", "2.5M Dir1"), ("5M Dir", "2.5M Dir2"),
    ("2.5M Esq1", "1.25M Esq1"), ("2.5M Esq1", "1.25M Esq2"),
    ("2.5M Esq2", "1.25M Esq3"), ("2.5M Esq2", "1.25M Esq4"),
    ("2.5M Dir1", "1.25M Dir1"), ("2.5M Dir1", "1.25M Dir2"),
    ("2.5M Dir2", "1.25M Dir3"), ("2.5M Dir2", "1.25M Dir4")
])

# Layout da árvore
pos = nx.nx_agraph.graphviz_layout(G, prog="dot")

plt.figure(figsize=(12, 8))
nx.draw(G, pos, with_labels=True, arrows=False, node_size=2500, node_color='lightblue', font_size=9, font_weight='bold')
plt.title("", fontsize=14)
plt.tight_layout()
plt.savefig('arvore_quicksort.png')
plt.show()
