tempos_execucao = [
    0.034168, 0.008874, 0.016800, 0.011587, 0.017641, 
    0.010034, 0.014611, 0.028681, 0.142844
]


# Calculando a média dos tempos de execução
media_tempos = sum(tempos_execucao) / len(tempos_execucao)

print(f"Média dos tempos de execução: {media_tempos:.6f} segundos")