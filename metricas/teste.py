#!/usr/bin/env python3
"""
Escalabilidade – 24 threads
======================================================
Gera:
  • Speed‑up, Eficiência, Overhead, Δt, f (Amdahl) e α (Gustafson)
  • Gráficos em ./figs/
  • relatorio_escalabilidade.txt com TODAS as métricas comentadas e
    recomendação automática da melhor linguagem.
"""

from pathlib import Path
import numpy as np, pandas as pd, matplotlib.pyplot as plt
from textwrap import dedent

# ─────────── Entradas ───────────
p = 24
n  = [1_000*i for i in range(1,11)]
C  = [2.3708,19.3422,65.6729,156.2970,304.6583,524.5770,833.6443,1240.4588,1764.6049,2288.7510]
Ct = [0.0496,0.3433,1.1507,2.7130,5.0142,8.6042,15.7853,26.7917,38.4549,53.4872]
Ju = [0.0087,0.0647,0.2112,0.5666,1.0712,1.8514,3.0741,4.4613,6.3644,8.8242]
OM = [0.1849,0.7190,1.8055,3.2564,7.1458,11.1366,16.4990,24.2513,36.1441,46.5580]

langs = {"C Threads":Ct,"Julia":Ju,"OpenMP":OM}

df = pd.DataFrame({"n":n,"C":C,**langs})

# ─────────── Cálculos ───────────
sp    = lambda t1,tp: t1/tp
E     = lambda s   : s/p
Ovh   = lambda tp,t1: p*tp - t1
famd  = lambda s   : (p*(1-1/s))/(p-1)
alpha = lambda s   : (p-s)/(p-1)

rows=[]
for L,tp in langs.items():
    t1          = np.asarray(C)
    tp          = np.asarray(tp)
    s           = sp(t1,tp)
    df[f"S ({L})"]=s;   df[f"E ({L})"]=E(s)
    df[f"O ({L})"]=Ovh(tp,t1); df[f"Dt ({L})"]=t1-tp
    df[f"f ({L})"] =famd(s);   df[f"a ({L})"] =alpha(s)
    rows.append(dict(Linguagem=L,
                     S_med=s.mean(),    E_med=E(s).mean(),
                     O_med=Ovh(tp,t1).mean(),
                     Dt_med=(t1-tp).mean(),
                     f_med=famd(s).mean(),
                     a_med=alpha(s).mean()))

res=pd.DataFrame(rows).set_index("Linguagem")

# ─────────── Gráficos ───────────
figs=Path("figs");figs.mkdir(exist_ok=True)
colors={"C Threads":"tab:orange","Julia":"tab:green","OpenMP":"tab:purple"}

def line(metric,col,y,log=False,ideal=False):
    plt.figure();
    for L in langs: plt.plot(df["n"],df[col.format(L)],"o-",label=L,color=colors[L])
    if ideal: plt.plot(df["n"],[p]*len(df),"k--",label=f"Ideal ({p}×)")
    plt.xlabel("Dimensão da matriz (n)");plt.ylabel(y);plt.title(metric)
    if log: plt.yscale("log"); plt.grid(ls="--",alpha=.4); plt.legend(); plt.tight_layout()
    plt.savefig(figs/f"{metric.replace(' ','_').lower()}.png",dpi=300); plt.close()

line("Speed‑up","S ({})","Speed‑up",ideal=True)
line("Eficiência","E ({})","Eficiência")
line("Overhead","O ({})","Overhead (s)",log=True)
line("Δt","Dt ({})","Δt (s)",log=True)
line("f (Amdahl)","f ({})","f")
line("α (Gustafson)","a ({})","α")

# ─────────── Relatório dinâmico ───────────
melhor=res.sort_values(["S_med","E_med","O_med"],ascending=[False,False,True]).index[0]

txt = dedent(f"""
    === Escalabilidade – 24 threads ===

    Tabela‑síntese (médias)
    -----------------------
{res.round(4).to_string()}

    Interpretação de cada métrica
    -----------------------------
    • **Speed‑up (Sₚ)** – fator de aceleração; quanto maior melhor.
    • **Eficiência (Eₚ = Sₚ/p)** – aproveitamento por núcleo; Eₚ≈1 é ideal, <0,3 indica desperdício.
    • **Overhead (p·Tₚ − T₁)** – tempo de CPU extra (positivo) ou economizado (negativo).
    • **Δt = T₁ − Tₚ** – segundos reais poupados.
    • **f (Amdahl)** – fração paralelizável (1 → escala linear; >1 indica speed‑up super‑linear).
    • **α (Gustafson)** – fração serial em escala fraca (0 é ideal; negativo quando há super‑linearidade).

    Resultados‑chave
    ----------------
    • Julia alcança Speed‑up médio {res.loc['Julia','S_med']:.1f}× e Eficiência média {res.loc['Julia','E_med']:.2f} (>1, super‑linear), com Overhead médio {res.loc['Julia','O_med']:.0f} s (negativo).
    • C Threads obtém Speed‑up {res.loc['C Threads','S_med']:.1f}×; Eficiência {res.loc['C Threads','E_med']:.2f}; Overhead {res.loc['C Threads','O_med']:.0f} s.
    • OpenMP fica em {res.loc['OpenMP','S_med']:.1f}×; Eficiência {res.loc['OpenMP','E_med']:.2f}; Overhead {res.loc['OpenMP','O_med']:.0f} s.
    • Fração paralelizável média (f): Julia {res.loc['Julia','f_med']:.3g}, C Threads {res.loc['C Threads','f_med']:.3g}, OpenMP {res.loc['OpenMP','f_med']:.3g}. Todas ≈1, logo o gargalo futuro é a memória. α segue a mesma tendência.

    Melhor escolha
    --------------
    **{melhor}** é recomendada, pois combina maior Speed‑up, alta Eficiência e Overhead mais favorável.

    Arquivos gerados
    ----------------
    • Gráficos: figs/*.png
    • Relatório: relatorio_escalabilidade.txt
""")

Path("relatorio_escalabilidade.txt").write_text(txt,encoding="utf-8")
print("✔ relatorio_escalabilidade.txt criado com todas as métricas comentadas.")
