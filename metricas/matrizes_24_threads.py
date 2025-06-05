#!/usr/bin/env python3
"""
Métricas de Variabilidade
=====================================

Este script calcula:

• Média (µ)
• Speed-up médio 
• Gráficos de violino (tempos) e boxplot (speed-up)

➔ Edite apenas a seção “DADOS DE ENTRADA”.
➔ Execute:  python metricas_variabilidade_confianca_inline.py
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

tamanhos = [1.000, 2.000, 3.000, 4.000, 5.000, 6.000, 7.000, 8.000, 9.000, 10.000]

tempos_C = [2.3708, 19.3422, 65.6729, 156.2970, 304.6583, 524.5770, 833.6443, 1240.4588, 1764.6049, 2288.7510]
tempos_C_threads = [0.0496, 0.3433, 1.1507, 2.7130, 5.0142, 8.6042, 15.7853, 26.7917, 38.4549, 53.4872]
tempos_julia = [0.0087, 0.0647, 0.2112, 0.5666, 1.0712, 1.8514, 3.0741, 4.4613, 6.3644, 8.8242]
tempos_OpenMP = [0.1849, 0.7190, 1.8055, 3.2564, 7.1458, 11.1366, 16.4990, 24.2513, 36.1441, 46.5580]
tempos_Halide = [0.000005, 0.000008, 0.000012, 0.000016, 0.000019, 0.000023, 0.000030, 0.000033, 0.000037, 0.000041]

em_ms = False

# -------------------------------------------------------------------------
# >>>>>>>>>>>>>>>>>>>>      FIM DA EDIÇÃO DOS DADOS     <<<<<<<<<<<<<<<<<<<<<<<<<<<<
# -------------------------------------------------------------------------

def estatisticas_por_linguagem(df: pd.DataFrame, sequencial: str = "C") -> pd.DataFrame:
    linguagens = [col for col in df.columns if col != "Tamanho"]
    linhas = []
    for lang in linguagens:
        tempos = df[lang]
        media, desvio = tempos.mean(), tempos.std(ddof=1)
        cov = desvio / media
        if lang != sequencial:
            speedup = df[sequencial] / tempos
            sp_media, sp_desvio = speedup.mean(), speedup.std(ddof=1)
            sp_cov = sp_desvio / sp_media
        else:
            sp_media = sp_desvio = sp_cov = np.nan
        linhas.append({"Linguagem": lang, "µ (s)": media, "µ Speed-up": sp_media})
    return pd.DataFrame(linhas).set_index("Linguagem")

def graficos(df: pd.DataFrame, pasta: Path):
    pasta.mkdir(exist_ok=True)
    linguagens = [col for col in df.columns if col != "Tamanho"]
    baseline = "C"

    fig, ax = plt.subplots()
    ax.violinplot([df[lang] for lang in linguagens], showmeans=True)
    ax.set_xticks(range(1, len(linguagens) + 1), linguagens)
    ax.set_ylabel("Tempo (s)")
    ax.set_title("Distribuição dos tempos por linguagem")
    fig.tight_layout()
    fig.savefig(pasta / "violino_tempos.png")

    speedups = {lang: df[baseline] / df[lang] for lang in linguagens if lang != baseline}
    fig, ax = plt.subplots()
    ax.boxplot(speedups.values(), labels=speedups.keys(), showmeans=True)
    ax.set_ylabel(f"Speed-up (vs {baseline})")
    ax.set_title("Speed-up por linguagem")
    fig.tight_layout()
    fig.savefig(pasta / "boxplot_speedup.png")

    fig, ax = plt.subplots()
    estilos = {
        "C": dict(color="black", marker="o", linestyle="--", linewidth=2),
        "C Threads": dict(color="tab:blue", marker="s", linestyle="-.", linewidth=1),
        "julia": dict(color="tab:orange", marker="^", linestyle="-", linewidth=1),
        "OpenMP": dict(color="tab:green", marker="d", linestyle="-", linewidth=1),
        "Halide": dict(color="tab:red", marker="*", linestyle="--", linewidth=1),
    }
    for lang in estilos:
        ax.plot(df["Tamanho"], df[lang], label=lang, **estilos[lang])
    ax.set_xlabel("Dimensão da matriz (n)")
    ax.set_ylabel("Tempo de execução (s)")
    ax.set_title("Tempo vs tamanho do problema")
    ax.set_yscale("log")
    ax.legend()
    fig.tight_layout()
    fig.savefig(pasta / "tempos_vs_n_todas_linguagens.png")

def gerar_relatorio_variabilidade(tabela: pd.DataFrame, destino: Path = Path("relatorio_variabilidade.txt")) -> None:
    linhas = []
    for lang, linha in tabela.iterrows():
        s = linha["µ Speed-up"]
        linhas.append(f"\u2022 {lang:<9} – tempo médio = {linha['µ (s)']:.4g} s"
                      f"{'' if pd.isna(s) else f',  speed-up médio = {s:.2f}×'}")
    melhor = tabela["µ Speed-up"].idxmax()
    melhor_val = tabela.loc[melhor, "µ Speed-up"]
    melhor_linha = (f"• A implementação **{melhor}** é a mais indicada, pois "
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
    dados = {
        "Tamanho": tamanhos,
        "C": tempos_C,
        "C Threads": tempos_C_threads,
        "julia": tempos_julia,
        "OpenMP": tempos_OpenMP,
        "Halide": tempos_Halide,
    }
    df = pd.DataFrame(dados)
    if em_ms:
        df.loc[:, df.columns != "Tamanho"] /= 1_000.0
    df.replace(0, np.nan, inplace=True)
    tabela = estatisticas_por_linguagem(df)

    print("\n=== Tempo médio por linguagem (µ) ===")
    for lang in tabela.index:
        print(f"• {lang:<9} → µ = {tabela.loc[lang, 'µ (s)']:.4f} s")

    print("\n=== Speed-up por medição (Sᵈ = Tseqᵈ / Timplementaçãoᵈ) ===")
    for lang in ["C Threads", "julia", "OpenMP", "Halide"]:
        speedups = df["C"] / df[lang]
        soma = 0.0
        print(f"\nSpeed-up para {lang}:")
        for i, s in enumerate(speedups, 1):
            t_seq = df["C"][i - 1]
            t_impl = df[lang][i - 1]
            soma += s
            print(f"• n={int(df['Tamanho'][i - 1])} → S_{i} = {t_seq:.4f} / {t_impl:.6f} = {s:.4f}×")
        media = soma / len(speedups)
        print(f"> Soma dos Sᵈ para {lang}: {soma:.4f}")
        print(f"> Média dos Sᵈ para {lang} (ᵀ̄): {media:.2f}×")

    print("\n=== Speed-up médio por linguagem (ᵀ̄) ===")
    for lang in tabela.index:
        s_med = tabela.loc[lang, "µ Speed-up"]
        if not pd.isna(s_med):
            print(f"• {lang:<9} → ᵀ̄ = {s_med:.2f}×")
            speedups = df["C"] / df[lang]
            for i, s in enumerate(speedups, 1):
                print(f"   S_{i} = {df['C'][i-1]:.4f} / {df[lang][i-1]:.6f} = {s:.4f}")
            soma = speedups.sum()
            print(f"   Soma dos Sᵈ = {soma:.4f}")
            print(f"   ᵀ̄ = Soma / n = {soma:.4f} / {len(speedups)} = {soma / len(speedups):.2f}×")

    print("\n=== Métricas de Variabilidade e Confiança ===")
    print(tabela.to_string(float_format="%.4g"))

    gerar_relatorio_variabilidade(tabela)
    graficos(df, Path("figs"))
    print("\nFiguras salvas em ./figs/.")

if __name__ == "__main__":
    main()
