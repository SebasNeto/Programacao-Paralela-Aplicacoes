tempos_execucao = [
    0.64, 2.63, 8.69, 22.14, 46.75, 
    80.86, 132.49, 227.84, 298.23, 414.54
]
# Calculando a média dos tempos de execução
media_tempos = sum(tempos_execucao) / len(tempos_execucao)

print(f"Média dos tempos de execução: {media_tempos:.6f} segundos")