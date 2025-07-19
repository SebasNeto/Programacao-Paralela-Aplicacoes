
import matplotlib.pyplot as plt
import numpy as np
import os

# Ordem das máquinas invertida
maquinas = ['4 threads', '24 threads']
x = np.arange(len(maquinas))
largura = 0.2

# Dados extraídos do documento (em milissegundos)
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
tecnicas = [
    ("Expansão de Contraste", python_exp, c_exp, halide_exp),
    ("Logaritmo", python_log, c_log, halide_log),
    ("Dente de Serra", python_dente, c_dente, halide_dente)
]

# Criar diretório de saída
output_dir = "graficos_transformadas"
os.makedirs(output_dir, exist_ok=True)

# Gerar gráficos separados
for nome, py, c, hl in tecnicas:
    fig, ax = plt.subplots(figsize=(8, 6))
    ax.bar(x - largura, py, largura, label='Python')
    ax.bar(x, c, largura, label='C')
    ax.bar(x + largura, hl, largura, label='Halide')
    ax.set_title(f'{nome} ')
    ax.set_xticks(x)
    ax.set_xticklabels(maquinas)
    ax.set_yscale('log')
    ax.set_ylabel('Tempo Médio (ms)')
    ax.set_xlabel('')
    ax.legend()
    plt.tight_layout()
    plt.savefig(f"{output_dir}/{nome.lower().replace(' ', '_')}.png", dpi=300, bbox_inches='tight')
    plt.close()

print(f"Gráficos salvos em: {output_dir}")
