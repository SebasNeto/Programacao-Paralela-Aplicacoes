import pandas as pd
import matplotlib.pyplot as plt
import os

# Dados de execução (tempos em milissegundos) – Filtragens Espaciais
dados_4_threads = pd.DataFrame({
    "Técnica": ["Média", "Mediana", "K Vizinhos"],
    "Python": [450812.63, 1105394.02, 918534.63],
    "C": [5138.11, 98179.71, 225874.53],
    "Halide": [442.13, 663.87, 1084.30]
})

dados_24_threads = pd.DataFrame({
    "Técnica": ["Média", "Mediana", "K Vizinhos"],
    "Python": [89158.42, 197033.25, 175001.33],
    "C": [1175.10, 20485.01, 42856.69],
    "Halide": [108.43, 167.20, 256.90]
})

# Função para plotar e salvar os gráficos
def plot_por_threads_e_salvar(df, titulo, filename, output_dir="graficos_filtragens"):
    os.makedirs(output_dir, exist_ok=True)

    fig, ax = plt.subplots(figsize=(10, 6))
    tecnicas = df["Técnica"]
    bar_width = 0.25
    x = range(len(tecnicas))

    ax.bar([i - bar_width for i in x], df["Python"], width=bar_width, label='Python')
    ax.bar(x, df["C"], width=bar_width, label='C')
    ax.bar([i + bar_width for i in x], df["Halide"], width=bar_width, label='Halide')

    ax.set_xticks(list(x))
    ax.set_xticklabels(tecnicas)
    ax.set_yscale('log')
    ax.set_ylabel("Tempo de Execução (ms)")
    ax.set_title(titulo)
    ax.legend()
    ax.grid(True, axis='y', linestyle='--', alpha=0.7)

    plt.tight_layout()
    path = os.path.join(output_dir, filename)
    plt.savefig(path)
    plt.close()
    print(f"Gráfico salvo em: {path}")

# Gerar e salvar os gráficos para filtragens espaciais
plot_por_threads_e_salvar(dados_4_threads, "Filtragens Espaciais - 4 Threads", "filtragens_4_threads.png")
plot_por_threads_e_salvar(dados_24_threads, "Filtragens Espaciais - 24 Threads", "filtragens_24_threads.png")
