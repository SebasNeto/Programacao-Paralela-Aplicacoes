tempos_execucao = [
    0.053624, 0.056901, 0.066432, 0.090156, 0.093722, 
    0.117323, 0.121615, 0.062750, 0.073949, 0.075139
]

# Calculando a média dos tempos de execução
media_tempos = sum(tempos_execucao) / len(tempos_execucao)

print(f"Média dos tempos de execução: {media_tempos:.6f} segundos")