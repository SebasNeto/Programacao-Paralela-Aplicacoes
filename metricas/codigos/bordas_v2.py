import pandas as pd
import matplotlib.pyplot as plt
import os

# Dados de execução (tempos em milissegundos) – Detecção de Bordas
dados_4_threads = pd.DataFrame({
    "Técnica": ["Sobel", "Roberts"],
    "Python": [786337.90, 816129.86],
    "C": [5574.52, 4750.93],
    "Halide": [1230.27, 1006.50]
})

dados_24_threads = pd.DataFrame({
    "Técnica": ["Sobel", "Roberts"],
    "Python": [163969.26, 161012.90],
    "C": [1367.59, 1048.42],
    "Halide": [260.17, 234.87]
})

# Função para plotar e salvar os gráficos
def plot_por_threads_e_salvar(df, titulo, filename, output_dir="graficos_bordas"):
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

# Gerar e salvar os gráficos para detecção de bordas
plot_por_threads_e_salvar(dados_4_threads, "Detecção de Bordas - 4 Threads", "bordas_4_threads.png")
plot_por_threads_e_salvar(dados_24_threads, "Detecção de Bordas - 24 Threads", "bordas_24_threads.png")
