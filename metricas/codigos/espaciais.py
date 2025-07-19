import matplotlib.pyplot as plt
import numpy as np
import os

# Ordem das máquinas: Desenvolvedor primeiro
maquinas = ['4 threads', '24 threads']
x = np.arange(len(maquinas))
largura = 0.2

# Dados extraídos do documento (em milissegundos)

# Técnica: Média
python_media = [450812.6250, 89158.4204]
c_media = [5138.1128, 1175.0984]
halide_media = [442.1333, 108.4333]

# Técnica: Mediana
python_mediana = [1105394.0205, 197033.2473]
c_mediana = [98179.7097, 20485.0092]
halide_mediana = [663.8667, 167.2000]

# Técnica: K Vizinhos Mais Próximos
python_kviz = [918534.6280, 175001.3283]
c_kviz = [225874.5325, 42856.6879]
halide_kviz = [1084.3000, 256.9000]

# Lista com dados por técnica
tecnicas = [
    ("Média", python_media, c_media, halide_media),
    ("Mediana", python_mediana, c_mediana, halide_mediana),
    ("K Vizinhos", python_kviz, c_kviz, halide_kviz)
]

# Criar diretório de saída
output_dir = "graficos_filtragens"
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
    ax.set_yscale('log')  # Escala logarítmica
    ax.set_ylabel('Tempo Médio (ms)')
    ax.set_xlabel('')
    ax.legend()
    plt.tight_layout()
    plt.savefig(f"{output_dir}/{nome.lower().replace(' ', '_').replace('+', 'mais')}.png", dpi=300, bbox_inches='tight')
    plt.close()

print(f"Gráficos salvos em: {output_dir}")
