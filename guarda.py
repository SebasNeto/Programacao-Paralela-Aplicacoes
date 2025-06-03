#!/usr/bin/env python3
"""
Escalabilidade
==============================================================

Calcula e plota:
• Speed-up Sₚ
• Eficiência Eₚ = Sₚ / p
• Overhead Oₚ = p·Tₚ − T₁
• Fração paralelizável (f) – Lei de Amdahl
• Fração serial (α) – Lei de Gustafson

Salva os gráficos em ./figs/.

"""

from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# -------------------------------------------------------------------------
# >>>>>>>>>>>>>>>>>>>>  DADOS DE ENTRADA  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
# -------------------------------------------------------------------------
p = 24  # número de threads da máquina

tamanhos = [1.000, 2.000, 3.000, 4.000, 5.000,6.000, 7.000, 8.000, 9.000, 10.000]

tempo_C = [2.3708, 19.3422, 65.6729, 156.2970, 304.6583, 524.5770, 833.6443, 1240.4588, 1764.6049, 2288.7510]

tempo_C_threads = [0.0496, 0.3433, 1.1507, 2.7130, 5.0142, 8.6042, 15.7853, 26.7917, 38.4549, 53.4872]

tempo_julia = [0.0087, 0.0647, 0.2112, 0.5666, 1.0712, 1.8514, 3.0741, 4.4613, 6.3644, 8.8242]

tempo_OpenMP = [0.1849, 0.7190, 1.8055, 3.2564, 7.1458, 11.1366, 16.4990, 24.2513, 36.1441, 46.5580]
# -------------------------------------------------------------------------

# --------------------------- FUNÇÕES AUXILIARES --------------------------
def speedup(t1: np.ndarray, tp: np.ndarray) -> np.ndarray:
    return t1 / tp

def eficiencia(sp: np.ndarray, p: int) -> np.ndarray:
    return sp / p

def overhead(tp: np.ndarray, t1: np.ndarray, p: int) -> np.ndarray:
    return p * tp - t1

def fracao_amdahl(sp: np.ndarray, p: int) -> np.ndarray:
    """f – parte paralelizável (Amdahl)."""
    return (p * (1 - 1 / sp)) / (p - 1)

def fracao_gustafson(sp: np.ndarray, p: int) -> np.ndarray:
    """α – parte serial (Gustafson)."""
    return (p - sp) / (p - 1)

# ------------------------- CONSTRUÇÃO DO DATAFRAME -----------------------
dados = {
    "Tamanho": tamanhos,
    "C": tempo_C,
    "C Threads": tempo_C_threads,
    "Julia": tempo_julia,
    "OpenMP": tempo_OpenMP,
}
df = pd.DataFrame(dados)

# ------------------------------ CÁLCULOS ---------------------------------
linguagens = ["C Threads", "Julia", "OpenMP"]
resultados = []

for lang in linguagens:
    tp = df[lang].to_numpy()
    t1 = df["C"].to_numpy()

    sp  = speedup(t1, tp)
    ep  = eficiencia(sp, p)
    op  = overhead(tp, t1, p)
    f_a = fracao_amdahl(sp, p)
    alfa_g = fracao_gustafson(sp, p)
    
    economia = t1 - tp                 # <<< NOVO: quantos segundos economizados
    df[f"Δt ({lang})"] = economia      # opcional: coluna no DataFrame

    df[f"Sₚ ({lang})"] = sp
    df[f"Eₚ ({lang})"] = ep
    df[f"Oₚ ({lang})"] = op
    df[f"f_Amdahl ({lang})"] = f_a
    df[f"α_Gustafson ({lang})"] = alfa_g

    resultados.append(
        {
            "Linguagem": lang,
            "Speed-up médio": sp.mean(),
            "Eficiência média": ep.mean(),
            "Overhead médio (s)": op.mean(),
            "Tempo economizado médio (s)": economia.mean(),
            "f (Amdahl) média": f_a.mean(),
            "α (Gustafson) média": alfa_g.mean(),
        }
    )

tabela = pd.DataFrame(resultados).set_index("Linguagem")
pd.options.display.float_format = "{:.4g}".format

# --------------------------- RELATÓRIO TEXTO -----------------------------
print("\n========== Escalabilidade – Máquina 24 threads ==========\n")
print(tabela)
print("\nInterpretação rápida:")
print("• Eficiência próxima de 1 ⇒ uso quase ideal dos núcleos; "
      "≤ 0,1 indica baixo aproveitamento.\n"
      "• Overhead positivo mede o tempo extra do paralelismo "
      "(sincronização, criação de threads, cache).\n"
      "• f (Amdahl) ≈ 1 e α (Gustafson) ≈ 0 sugerem algoritmo altamente paralelizável.\n")

# ------------------------------ GRÁFICOS ---------------------------------
pasta = Path("figs")
pasta.mkdir(exist_ok=True)

cores = {"C Threads": "tab:orange", "Julia": "tab:green", "OpenMP": "tab:purple"}

def plot_linha(col_padrao: str, titulo: str, ylabel: str, nome_arq: str, logy=False):
    plt.figure()
    for lang in linguagens:
        plt.plot(df["Tamanho"], df[col_padrao.format(lang)], marker="o",
                 label=lang, color=cores[lang])
    plt.xlabel("Tamanho da matriz")
    plt.ylabel(ylabel)
    plt.title(titulo)
    if logy:
        plt.yscale("log")
    plt.legend()
    plt.grid(True, ls="--", alpha=0.5)
    plt.tight_layout()
    plt.savefig(pasta / nome_arq, dpi=300)

# speed-up
plot_linha("Sₚ ({})",
           "Speed-up vs. Tamanho (24 threads)",
           "Speed-up", "speedup.png")

# eficiência
plot_linha("Eₚ ({})",
           "Eficiência vs. Tamanho (24 threads)",
           "Eficiência Eₚ", "eficiencia.png")

# overhead (escala log para melhor visualização)
plot_linha("Oₚ ({})",
           "Overhead vs. Tamanho (24 threads)",
           "Overhead Oₚ (s)", "overhead.png", logy=True)

# fração Amdahl
plot_linha("f_Amdahl ({})",
           "Fração paralelizável f – Lei de Amdahl",
           "f", "amdahl_f.png")

# fração Gustafson
plot_linha("α_Gustafson ({})",
           "Fração serial α – Lei de Gustafson",
           "α", "gustafson_alpha.png")

plot_linha("Δt ({})",
           "Tempo economizado Δt vs. Tamanho",
           "Δt (s)", "delta_t.png")


print("Gráficos salvos em ./figs/")

#matrizes original
#!/usr/bin/env python3
"""
Métricas de Variabilidade
====================================

Este script calcula:

• Média (µ)
• Speed-up médio 
• Gráficos de violino (tempos) e boxplot (speed-up)

➜ Edite apenas a seção “DADOS DE ENTRADA”.
➜ Execute:  python metricas_variabilidade_confianca_inline.py
           (gera a tabela no terminal e figuras em ./figs/)
"""

from pathlib import Path
from datetime import datetime, time

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import scipy.stats as sts

# -------------------------------------------------------------------------
# >>>>>>>>>>>>>>>>>>>>   DADOS DE ENTRADA <<<<<<<<<<<<<<<<<<
# -------------------------------------------------------------------------

# Tamanhos dos problemas (dimensão das matrizes)
tamanhos = [1.000, 2.000, 3.000, 4.000, 5.000,6.000, 7.000, 8.000, 9.000, 10.000]

# Tempos medidos para *cada* tamanho (na mesma ordem de 'tamanhos')
# Use milissegundos se 'em_ms' = True, ou segundos se 'em_ms' = False

tempos_C          = [2.4034, 20.9721, 68.2422, 159.4918, 312.8576, 540.7934, 851.1212, 1267.4667, 1810.9632, 2497.9901]
tempos_C_threads  = [0.1803, 0.9484, 2.5847, 6.3103, 12.2839, 20.9787, 34.2099, 52.4907, 70.2387, 106.6906]
tempos_julia      = [0.0094, 0.0787, 0.3130, 0.6049, 1.2446, 2.4135, 3.4858, 5.4525, 8.1530, 11.4206]
tempos_OpenMP     = [0.2702, 1.0467, 2.4632, 8.3562, 13.2729, 20.6622, 40.5295, 52.8999, 68.9883, 112.4239]

# Os tempos acima estão em milissegundos?
em_ms = False

# -------------------------------------------------------------------------
# >>>>>>>>>>>>>>>>>>>>      FIM DA EDIÇÃO DOS DADOS     <<<<<<<<<<<<<<<<<<<<<<<<<<<<
# -------------------------------------------------------------------------


# def intervalo_confianca_95(serie: pd.Series) -> tuple[float, float]:
#     """Calcula o IC de 95 % para a média de uma série numérica."""
#     n = serie.count()
#     media = serie.mean()
#     erro_padrao = serie.std(ddof=1) / np.sqrt(n)
#     margem = sts.t.ppf(0.975, df=n - 1) * erro_padrao
#     return media - margem, media + margem


def estatisticas_por_linguagem(df: pd.DataFrame, sequencial: str = "C") -> pd.DataFrame:
    """Gera tabela com métricas de variabilidade e confiança por linguagem."""
    linguagens = [col for col in df.columns if col != "Tamanho"]
    linhas = []

    for lang in linguagens:
        tempos = df[lang]
        media, desvio = tempos.mean(), tempos.std(ddof=1)
        cov = desvio / media
        #ic_baixo, ic_alto = intervalo_confianca_95(tempos)

        if lang != sequencial:
            speedup = df[sequencial] / tempos
            sp_media, sp_desvio = speedup.mean(), speedup.std(ddof=1)
            sp_cov = sp_desvio / sp_media
            #sp_ic_baixo, sp_ic_alto = intervalo_confianca_95(speedup)
        else:
            sp_media = sp_desvio = sp_cov = sp_ic_baixo = sp_ic_alto = np.nan

        linhas.append(
            {
                "Linguagem": lang,
                "µ (s)": media,
                "µ Speed-up": sp_media,
            }
        )

    return pd.DataFrame(linhas).set_index("Linguagem")


def graficos(df: pd.DataFrame, pasta: Path):
    """Cria e salva gráficos de distribuição dos tempos e speed-ups."""
    pasta.mkdir(exist_ok=True)
    linguagens = [col for col in df.columns if col != "Tamanho"]
    baseline = "C"

    # Violino dos tempos
    fig, ax = plt.subplots()
    ax.violinplot([df[lang] for lang in linguagens], showmeans=True)
    ax.set_xticks(range(1, len(linguagens) + 1), linguagens)
    ax.set_ylabel("Tempo (s)")
    ax.set_title("Distribuição dos tempos por linguagem")
    fig.tight_layout()
    fig.savefig(pasta / "violino_tempos.png")

    # Boxplot dos speed-ups
    speedups = {lang: df[baseline] / df[lang] for lang in linguagens if lang != baseline}
    fig, ax = plt.subplots()
    ax.boxplot(speedups.values(), labels=speedups.keys(), showmeans=True)
    ax.set_ylabel(f"Speed-up (vs {baseline})")
    ax.set_title("Speed-up por linguagem")
    fig.tight_layout()
    fig.savefig(pasta / "boxplot_speedup.png")
    
    # --- curva Tempo × Dimensão (todas as linguagens) ----------------------
    fig, ax = plt.subplots()

    #cores e estilos fixos para facilitar leitura
    estilos = {
        "C":          dict(color="black",   marker="o", linestyle="--",  linewidth=2),
        "C Threads":  dict(color="tab:blue",marker="s", linestyle="-.", linewidth=1),
        "julia":      dict(color="tab:orange",marker="^", linestyle="-", linewidth=1),
        "OpenMP":     dict(color="tab:green",marker="d", linestyle="-",  linewidth=1),
    }

    # plota cada coluna num mesmo eixo
    for lang in ["C", "C Threads", "julia", "OpenMP"]:
        ax.plot(df["Tamanho"], df[lang], label=lang, **estilos[lang])

    ax.set_xlabel("Dimensão da matriz (n)")
    ax.set_ylabel("Tempo de execução (s)")
    ax.set_title("Tempo vs tamanho de problema — todas as linguagens")
    ax.set_yscale("log")         # escala log no eixo Y
    # ax.set_xscale("log")       # habilite se quiser log-log
    ax.legend()
    fig.tight_layout()
    fig.savefig(pasta / "tempos_vs_n_todas_linguagens.png")

def gerar_relatorio_variabilidade(tabela: pd.DataFrame,
                                  destino: Path = Path("relatorio_variabilidade.txt")) -> None:
    """
    Gera um arquivo-texto com a síntese das métricas de variabilidade
    e confiança (µ, speed-up médio, dispersão) **e indica automaticamente
    a linguagem com melhor desempenho**.

    Parâmetros
    ----------
    tabela : DataFrame
        Saída de `estatisticas_por_linguagem(df)`.
    destino : Path
        Caminho do .txt a ser gravado.
    """
    # ­­ lista textual das linguagens
    linhas = []
    for lang, linha in tabela.iterrows():
        s = linha["µ Speed-up"]
        linhas.append(f"• {lang:<9} – tempo médio = {linha['µ (s)']:.4g} s"
                      f"{'' if pd.isna(s) else f',  speed-up médio = {s:.2f}×'}")

    # ­­ MELHOR IMPLEMENTAÇÃO (maior speed-up médio)
    melhor        = tabela["µ Speed-up"].idxmax()
    melhor_val    = tabela.loc[melhor, "µ Speed-up"]
    melhor_linha  = (f"• A implementação **{melhor}** é a mais indicada, pois "
                     f"exibe o maior speed-up médio (≈ {melhor_val:.2f}×) "
                     "em relação à versão sequencial C.\n")

    corpo = f"""
    === Variabilidade & Confiança ===

    Tabela-síntese
    -------------- 
    {tabela.to_string()}

    Interpretação rápida
    --------------------
    {chr(10).join(linhas)}
    {melhor_linha}
    • O boxplot de speed-up mostra a dispersão: caixas estreitas indicam
      estabilidade; outliers sugerem interferência do SO ou cache.
    • Se o coeficiente de variação (σ/µ) superar 10 %, recomenda-se aumentar
      as repetições ou isolar ruído de background antes de conclusões
      definitivas.

    Arquivos gerados
    ----------------
    • Gráficos: figs/violino_tempos.png, figs/boxplot_speedup.png,
                figs/tempos_vs_n_todas_linguagens.png
    • Relatório: {destino.name}
    """.strip()

    destino.write_text(corpo, encoding="utf-8")
    print(f"✔  Relatório salvo em {destino}")


def main():
    # Construção do DataFrame
    dados = {
        "Tamanho": tamanhos,
        "C": tempos_C,
        "C Threads": tempos_C_threads,
        "julia": tempos_julia,
        "OpenMP": tempos_OpenMP,
    }
    df = pd.DataFrame(dados)

    # Converte ms → s, se necessário
    if em_ms:
        df.loc[:, df.columns != "Tamanho"] /= 1_000.0

    # Substitui zeros por NaN para evitar speed-up infinito
    df.replace(0, np.nan, inplace=True)

    tabela = estatisticas_por_linguagem(df)
    print("\n=== Métricas de Variabilidade e Confiança ===")
    print(tabela.to_string(float_format="%.4g"))
    
    # Imprime o Speed-up por medição individual (linha a linha)
    print("\n=== Speed-up por medição (Sᵢ = Tseqᵢ / Timplementaçãoᵢ) ===")
    for lang in ["C Threads", "julia", "OpenMP"]:
        speedups = df["C"] / df[lang]
        print(f"\nSpeed-up para {lang}:")
        for i, s in enumerate(speedups, 1):
            print(f"• n={int(df['Tamanho'][i-1])} → S_{i} = {s:.2f}×")
    
    gerar_relatorio_variabilidade(tabela)
    
    graficos(df, Path("figs"))
    print("\nFiguras salvas em ./figs/.")


if __name__ == "__main__":
    main()


#funções isoladas

def exibir_tempo_medio(df: pd.DataFrame, tabela: pd.DataFrame) -> None:
    print("\n=== Tempo médio por linguagem (µ) ===")
    for lang in tabela.index:
        tempo = tabela.loc[lang, "µ (s)"]
        print(f"• {lang:<9} → µ = {tempo:.4f} s")


def exibir_speedup_por_medicao(df: pd.DataFrame) -> None:
    print("\n=== Speed-up por medição (Sᵢ = Tseqᵢ / Timplementaçãoᵢ) ===")
    for lang in ["C Threads", "julia", "OpenMP"]:
        speedups = df["C"] / df[lang]
        print(f"\nSpeed-up para {lang}:")
        for i, s in enumerate(speedups, 1):
            print(f"• n={int(df['Tamanho'][i-1])} → S_{i} = {s:.2f}×")


def exibir_speedup_medio(tabela: pd.DataFrame) -> None:
    print("\n=== Speed-up médio por linguagem (S̄) ===")
    for lang in tabela.index:
        s_med = tabela.loc[lang, "µ Speed-up"]
        if not pd.isna(s_med):
            print(f"• {lang:<9} → S̄ = {s_med:.2f}×")


def main():
    # Construção do DataFrame
    dados = {
        "Tamanho": tamanhos,
        "C": tempos_C,
        "C Threads": tempos_C_threads,
        "julia": tempos_julia,
        "OpenMP": tempos_OpenMP,
    }
    df = pd.DataFrame(dados)

    # Converte ms → s, se necessário
    if em_ms:
        df.loc[:, df.columns != "Tamanho"] /= 1_000.0

    df.replace(0, np.nan, inplace=True)  # Evita divisão por zero

    tabela = estatisticas_por_linguagem(df)

    # Exibe métricas em ordem
    exibir_tempo_medio(df, tabela)
    exibir_speedup_por_medicao(df)
    exibir_speedup_medio(tabela)

    # Exibe tabela final e gera relatório/gráficos
    print("\n=== Métricas de Variabilidade e Confiança ===")
    print(tabela.to_string(float_format="%.4g"))
    gerar_relatorio_variabilidade(tabela)
    graficos(df, Path("figs"))
    print("\nFiguras salvas em ./figs/.")
