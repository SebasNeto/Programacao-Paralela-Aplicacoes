import matplotlib.pyplot as plt
import numpy as np

# Dados dos speedups
speedup_C_threads = [7.1514, 7.1504, 7.0923, 7.2527, 6.9147, 7.0496, 7.1013, 7.1469, 7.1241, 6.7582]
speedup_Julia = [260.0081, 257.4861, 257.2727, 248.6654, 245.6510, 225.3279, 211.1321, 226.9290, 212.1719, 215.1401]
speedup_OpenMP = [9.0623, 15.0925, 19.1776, 24.0868, 28.1314, 29.0224, 31.5274, 31.5464, 26.6370, 26.9752]
speedup_Halide = [35933.7079, 154491.6667, 355182.1862, 645221.3622, 992899.7549,
                  1440236.5702, 1978798.0357, 2590973.8245, 3311549.8596, 3862050.0631]

tamanhos = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

# Criando gráfico com eixo secundário
fig, ax1 = plt.subplots(figsize=(10, 7))

# Plotando C Threads, OpenMP, Julia no eixo primário
ax1.set_xlabel('Dimensão da matriz (n)')
ax1.set_ylabel('Speed-up (C Threads, OpenMP, Julia)')
ax1.plot(tamanhos, speedup_C_threads, 'bs-.', label='C Threads')
ax1.plot(tamanhos, speedup_OpenMP, 'gD-', label='OpenMP')
ax1.plot(tamanhos, speedup_Julia, '^-', color='orange', label='Julia')
ax1.tick_params(axis='y')
ax1.grid(True, linestyle='--', linewidth=0.5)

# Eixo secundário para Halide
ax2 = ax1.twinx()
ax2.set_ylabel('Speed-up Halide')
ax2.plot(tamanhos, speedup_Halide, 'r*-', label='Halide')
ax2.tick_params(axis='y', colors='red')

# Unindo as legendas dos dois eixos
lns1 = ax1.get_lines()
lns2 = ax2.get_lines()
labels = [l.get_label() for l in lns1 + lns2]
ax1.legend(lns1 + lns2, labels, loc='upper left')

plt.title('Speed-up vs tamanho do problema (com Halide em eixo secundário)')
plt.tight_layout()
plt.show()

