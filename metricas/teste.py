import matplotlib.pyplot as plt
import numpy as np

# Dados
tamanhos = [1.000, 2.000, 3.000, 4.000, 5.000, 6.000, 7.000, 8.000, 9.000, 10.000]

tempos_C = [2.3708, 19.3422, 65.6729, 156.2970, 304.6583, 524.5770, 833.6443, 1240.4588, 1764.6049, 2288.7510]

tempos_C_threads = [0.0496, 0.3433, 1.1507, 2.7130, 5.0142, 8.6042, 15.7853, 26.7917, 38.4549, 53.4872]

tempos_julia = [0.0087, 0.0647, 0.2112, 0.5666, 1.0712, 1.8514, 3.0741, 4.4613, 6.3644, 8.8242]

tempos_OpenMP = [0.1849, 0.7190, 1.8055, 3.2564, 7.1458, 11.1366, 16.4990, 24.2513, 36.1441, 46.5580]

#tempos_Halide = [0.000005, 0.000008, 0.000012, 0.000016, 0.000019, 0.000023, 0.000030, 0.000033, 0.000037, 0.000041]
tempos_Halide= [0.0251, 0.2107, 0.7129, 1.6738, 3.2057, 5.4051, 8.5143, 12.6149, 17.9128, 24.5083]


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
print(f"• C: {tempo_medio_C:.6f} s")
print(f"• C Threads: {tempo_medio_C_threads:.6f} s")
print(f"• Julia: {tempo_medio_Julia:.6f} s")
print(f"• OpenMP: {tempo_medio_OpenMP:.6f} s")
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