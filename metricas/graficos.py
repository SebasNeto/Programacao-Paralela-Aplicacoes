import matplotlib.pyplot as plt
import numpy as np

# Dados
maquinas = ['6 Threads', '16 Threads', '24 Threads']
tempos_C = [262.9244, 145.8125, 108.8022]
tempos_C_threads = [136.9614, 102.8712, 106.5027]
tempos_Julia = [ 359.0624, 346.0395, 203.7626]
tempos_OpenMP = [168.2069, 101.9961,80.2740]
tempos_Halide = [0.6176, 0.5826, 0.1182]

# Configuração do gráfico
x = np.arange(len(maquinas))
largura = 0.15

fig, ax = plt.subplots(figsize=(10, 6))
rects1 = ax.bar(x - 2*largura, tempos_C, largura, label='C')
rects2 = ax.bar(x - largura, tempos_C_threads, largura, label='C Threads')
rects3 = ax.bar(x, tempos_Julia, largura, label='Julia')
rects4 = ax.bar(x + largura, tempos_OpenMP, largura, label='OpenMP')
rects5 = ax.bar(x + 2*largura, tempos_Halide, largura, label='Halide')

# Adiciona labels
ax.set_ylabel('Tempo Médio (s)')
ax.set_title('')
ax.set_xticks(x)
ax.set_xticklabels(maquinas)
ax.legend()
ax.set_yscale('log')  # Escala logarítmica para melhor visualização

plt.tight_layout()
#plt.show()
plt.savefig('otsu.png', dpi=300, bbox_inches='tight')
