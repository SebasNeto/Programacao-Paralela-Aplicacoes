import pandas as pd
import matplotlib.pyplot as plt
import os

# Dados de execução (tempos em milissegundos)
dados_4_threads = pd.DataFrame({
    "Técnica": ["Expansão Linear", "Logaritmo", "Dente de Serra"],
    "Python": [10540.8364, 7649.6159, 23112.5866],
    "C": [272.8832, 421.8791, 260.5805],
    "Halide": [185.4667, 211.5000, 189.3667]
})

dados_24_threads = pd.DataFrame({
    "Técnica": ["Expansão Linear", "Logaritmo", "Dente de Serra"],
    "Python": [10540.8364, 7649.6159, 23112.5866],
    "C": [108.4604, 154.5248, 121.7629],
    "Halide": [52.2667, 57.9667, 54.6000]
})

# Função para plotar e salvar os gráficos
def plot_por_threads_e_salvar(df, titulo, filename, output_dir="graficos_transformacoes"):
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
    ax.set_ylabel("Tempo de Execução (ms) - Escala Log")
    ax.set_title(titulo)
    ax.legend()
    ax.grid(True, axis='y', linestyle='--', alpha=0.7)

    plt.tight_layout()
    path = os.path.join(output_dir, filename)
    plt.savefig(path)
    plt.close()
    print(f"Gráfico salvo em: {path}")

# Gerar e salvar os gráficos
plot_por_threads_e_salvar(dados_4_threads, "Transformações Radiométricas - 4 Threads", "transformacoes_4_threads.png")
plot_por_threads_e_salvar(dados_24_threads, "Transformações Radiométricas - 24 Threads", "transformacoes_24_threads.png")
