import matplotlib.pyplot as plt
import numpy as np

# Dados
tamanhos = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
           11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
           21, 22, 23, 24, 25, 26, 27, 28, 29, 30]


tempos_C = [
    15.8669, 13.4379, 13.6636, 20.7729, 11.7842,
    18.7703, 15.3479, 25.4766, 21.3360, 23.7751,
    21.9041, 44.7659, 62.3584, 45.9801, 106.6244,
    71.9772, 71.4759, 65.8377, 67.2688, 176.4803,
    146.7719, 115.9576, 131.9672, 237.6337, 196.1409,
    196.0453, 275.8452, 355.3018, 329.7963, 363.7022
]


tempos_C_threads = [
    15.7320, 16.5350, 14.1290, 20.2200, 13.7800,
    20.0660, 16.3330, 25.8850, 22.3480, 26.4270,
    24.2490, 46.7660, 58.2830, 46.1150, 91.2570,
    69.1150, 69.9410, 68.7770, 73.3140, 155.2740,
    134.5330, 122.9080, 135.9020, 232.0410, 193.7850,
    209.3370, 256.5470, 330.6980, 315.9790, 368.8050
]

tempos_julia = [
    16.1650, 20.7119, 27.0810, 34.4110, 18.9290,
    25.1539, 24.1752, 36.2060, 38.0530, 111.1941,
    40.7240, 82.0730, 97.8830, 88.3009, 219.1091,
    133.7831, 145.7279, 204.9501, 145.7472, 262.7470,
    299.4261, 232.2180, 245.2168, 437.2230, 362.2830,
    412.4489, 488.6599, 530.6332, 656.1501, 675.4940
]


tempos_OpenMP = [
    14.0962, 11.4914, 10.9044, 15.0182, 4.9905,
    12.3642, 12.7584, 16.4953, 10.0172, 13.7396,
    16.3058, 29.0346, 39.4385, 25.5178, 58.8428,
    44.8120, 34.6991, 38.4665, 33.1411, 132.9738,
    116.9284, 87.9508, 106.7164, 198.5039, 145.0387,
    152.3978, 203.7144, 279.0577, 249.2090, 293.5968
]

tempos_Halide =  [
    0.1057, 0.1059, 0.1070, 0.1059, 0.1067,
    0.1067, 0.1074, 0.1179, 0.1080, 0.1083,
    0.1110, 0.1096, 0.1122, 0.1140, 0.1088,
    0.1161, 0.1178, 0.1185, 0.1198, 0.1202,
    0.1212, 0.1245, 0.1263, 0.1276, 0.1279,
    0.1330, 0.1333, 0.1366, 0.1422, 0.1452
]


# ---------------- Funções ----------------

def calcular_tempo_medio(tempos):
    return np.mean(tempos)


def calcular_speedup_por_medicao(tempos_seq, tempos_paralelo, nome):
    print(f"\nSpeed-up para {nome}:")
    speedups = []
    soma = 0
    for i in range(len(tempos_seq)):
        s = tempos_seq[i] / tempos_paralelo[i]
        soma += s
        speedups.append(s)
        print(f"• n={int(tamanhos[i])} → S_{i+1} = {tempos_seq[i]:.4f} / {tempos_paralelo[i]:.6f} = {s:.4f}×")
    media = soma / len(tempos_seq)
    print(f"> Soma dos Sᵈ para {nome}: {soma:.4f}")
    print(f"> Média dos Sᵈ para {nome} (ᵀ̄): {media:.2f}×")
    return speedups, media


def calcular_speedup_medio(tempo_medio_seq, tempo_medio_par, nome):
    s = tempo_medio_seq / tempo_medio_par
    print(f"• {nome}: {tempo_medio_seq:.4f} / {tempo_medio_par:.6f} = {s:.2f}×")
    return s

# ---------------- Speed-up por medição ----------------

speedup_C_threads, _ = calcular_speedup_por_medicao(tempos_C, tempos_C_threads, "C Threads")
speedup_Julia, _ = calcular_speedup_por_medicao(tempos_C, tempos_julia, "Julia")
speedup_OpenMP, _ = calcular_speedup_por_medicao(tempos_C, tempos_OpenMP, "OpenMP")
speedup_Halide, _ = calcular_speedup_por_medicao(tempos_C, tempos_Halide, "Halide")

# ---------------- Tempo médio ----------------

tempo_medio_C = calcular_tempo_medio(tempos_C)
tempo_medio_C_threads = calcular_tempo_medio(tempos_C_threads)
tempo_medio_Julia = calcular_tempo_medio(tempos_julia)
tempo_medio_OpenMP = calcular_tempo_medio(tempos_OpenMP)
tempo_medio_Halide = calcular_tempo_medio(tempos_Halide)

print("\nTempo médio de execução:")
print(f"• C: {tempo_medio_C:.4f} s")
print(f"• C Threads: {tempo_medio_C_threads:.4f} s")
print(f"• Julia: {tempo_medio_Julia:.4f} s")
print(f"• OpenMP: {tempo_medio_OpenMP:.4f} s")
print(f"• Halide: {tempo_medio_Halide:.6f} s")

# ---------------- Speed-up baseado no tempo médio ----------------

print("\nSpeed-up baseado no tempo médio:")
speedup_medio_C_threads = calcular_speedup_medio(tempo_medio_C, tempo_medio_C_threads, "C Threads")
speedup_medio_Julia = calcular_speedup_medio(tempo_medio_C, tempo_medio_Julia, "Julia")
speedup_medio_OpenMP = calcular_speedup_medio(tempo_medio_C, tempo_medio_OpenMP, "OpenMP")
speedup_medio_Halide = calcular_speedup_medio(tempo_medio_C, tempo_medio_Halide, "Halide")

# ---------------- Gráfico ----------------

plt.figure(figsize=(10, 7))
plt.yscale('log')

plt.plot(tamanhos, tempos_C, 'ko--', label='C')
plt.plot(tamanhos, tempos_C_threads, 'bs-.', label='C Threads')
plt.plot(tamanhos, tempos_julia, '^-', color='orange', label='Julia')
plt.plot(tamanhos, tempos_OpenMP, 'gD-', label='OpenMP')
plt.plot(tamanhos, tempos_Halide, 'r*-', label='Halide')

plt.xlabel('Dimensão (n)')
plt.ylabel('Tempo de execução (s)')
plt.title('')
plt.legend()
plt.grid(True, which="both", linestyle='--', linewidth=0.5)
plt.tight_layout()
plt.show()

################## gráfico barras speedup médio ##################
speedups_medios = [speedup_medio_C_threads, speedup_medio_Julia, speedup_medio_OpenMP, speedup_medio_Halide]
labels = ['C Threads', 'Julia', 'OpenMP', 'Halide']

plt.figure(figsize=(10, 7))
plt.bar(labels, speedups_medios, color=['blue', 'orange', 'green', 'red'])
plt.yscale('log')

plt.title('')
plt.ylabel('Speed-up (escala logarítmica)')
plt.xlabel('Abordagem')

for i, v in enumerate(speedups_medios):
    plt.text(i, v * 1.1, f'{v:.2f}×', ha='center', fontsize=10)

plt.grid(axis='y', which="both", linestyle='--', linewidth=0.5)
plt.tight_layout()
plt.show()