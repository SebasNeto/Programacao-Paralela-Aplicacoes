import matplotlib.pyplot as plt
import numpy as np

# Ordem das máquinas invertida
maquinas = ['Desenvolvedor', 'ICOMP']
x = np.arange(len(maquinas))
largura = 0.2

# Dados reorganizados conforme a nova ordem

# Técnica 1: Expansão de Contraste Linear
python_exp = [54202.6059, 10540.8364]
c_exp = [272.8832, 108.4604]
halide_exp = [185.4667, 52.2667]

# Técnica 2: Logaritmo
python_log = [40204.0658, 7649.6159]
c_log = [421.8791, 154.5248]
halide_log = [211.5000, 57.9667]

# Técnica 3: Dente de Serra
python_dente = [119273.2424, 23112.5866]
c_dente = [260.5805, 121.7629]
halide_dente = [189.3667, 54.6000]

# Lista com dados por técnica
tecnicas = ['Expansão de Contraste', 'Logaritmo', 'Dente de Serra']
dados = [
    (python_exp, c_exp, halide_exp),
    (python_log, c_log, halide_log),
    (python_dente, c_dente, halide_dente)
]

# Criação dos subplots
fig, axs = plt.subplots(1, 3, figsize=(18, 6), sharey=True)

for i, (py, c, hl) in enumerate(dados):
    axs[i].bar(x - largura, py, largura, label='Python')
    axs[i].bar(x, c, largura, label='C')
    axs[i].bar(x + largura, hl, largura, label='Halide')
    axs[i].set_title(tecnicas[i])
    axs[i].set_xticks(x)
    axs[i].set_xticklabels(maquinas)
    axs[i].set_yscale('log')
    axs[i].set_xlabel('Máquina')
    if i == 0:
        axs[i].set_ylabel('Tempo Médio (ms)')

# Legenda geral
handles, labels = axs[0].get_legend_handles_labels()
fig.legend(handles, labels, loc='upper center', ncol=3)

plt.suptitle('')
plt.tight_layout(rect=[0, 0, 1, 0.92])
plt.savefig('transformadas.png', dpi=300, bbox_inches='tight')
# plt.show()