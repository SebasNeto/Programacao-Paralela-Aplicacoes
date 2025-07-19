import matplotlib.pyplot as plt
import numpy as np
import os

# Ordem das máquinas: Desenvolvedor primeiro
maquinas = ['4 threads', '24 threads']
x = np.arange(len(maquinas))
largura = 0.2

# Dados extraídos do documento (em milissegundos)

# Técnica: Sobel
python_sobel = [786337.9029, 163969.2557]
c_sobel = [5574.5200, 1367.5888]
halide_sobel = [1230.2667, 260.1667]

# Técnica: Roberts
python_roberts = [816129.8633, 161012.9039]
c_roberts = [4750.9302, 1048.4216]
halide_roberts = [1006.5000, 234.8667]

# Lista com dados por técnica
tecnicas = [
    ("Sobel", python_sobel, c_sobel, halide_sobel),
    ("Roberts", python_roberts, c_roberts, halide_roberts)
]

# Criar diretório de saída
output_dir = "graficos_bordas"
os.makedirs(output_dir, exist_ok=True)

# Gerar gráficos separados
for nome, py, c, hl in tecnicas:
    fig, ax = plt.subplots(figsize=(8, 6))
    ax.bar(x - largura, py, largura, label='Python')
    ax.bar(x, c, largura, label='C')
    ax.bar(x + largura, hl, largura, label='Halide')
    ax.set_title(f'{nome}')
    ax.set_xticks(x)
    ax.set_xticklabels(maquinas)
    ax.set_yscale('log')  # Escala logarítmica para visualizar melhor
    ax.set_ylabel('Tempo Médio (ms)')
    ax.set_xlabel('')
    ax.legend()
    plt.tight_layout()
    plt.savefig(f"{output_dir}/{nome.lower()}.png", dpi=300, bbox_inches='tight')
    plt.close()

print(f"Gráficos salvos em: {output_dir}")
