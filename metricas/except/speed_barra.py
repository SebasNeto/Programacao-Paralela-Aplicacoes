# Gerando gráfico de barras com os speed-ups médios (globais)
import matplotlib.pyplot as plt
import numpy as np

# Valores dos speed-ups médios (calculados anteriormente)
speedups_medios = [6.99, 218.81, 27.99, 2173013.16]
labels = ['C Threads', 'Julia', 'OpenMP', 'Halide']

plt.figure(figsize=(10, 7))
plt.bar(labels, speedups_medios, color=['blue', 'orange', 'green', 'red'])
plt.yscale('log')

plt.title('Speed-up médio (global) por abordagem')
plt.ylabel('Speed-up (escala logarítmica)')
plt.xlabel('Abordagem')

for i, v in enumerate(speedups_medios):
    plt.text(i, v * 1.1, f'{v:.2f}×', ha='center', fontsize=10)

plt.grid(axis='y', which="both", linestyle='--', linewidth=0.5)
plt.tight_layout()
plt.show()
