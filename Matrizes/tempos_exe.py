tempos_execucao = [
    0.4294, 0.4399, 0.4319, 0.4199, 0.4372,
    0.4290, 0.4357, 0.4274, 0.4513, 0.4370,
    0.4753, 0.5027, 0.5018, 0.5341, 0.5426,
    0.5808, 0.5783, 0.5603, 0.5955, 0.5855,
    0.6029, 0.7043, 0.6452, 0.7035, 0.6665,
    0.7844, 0.7459, 0.7768, 1.0205, 1.0319
]


# Calculando a média dos tempos de execução
media_tempos = sum(tempos_execucao) / len(tempos_execucao)

print(f"Média dos tempos de execução: {media_tempos:.4f} segundos")