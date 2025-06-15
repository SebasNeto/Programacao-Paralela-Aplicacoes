tempos_execucao = [15.4339, 45.2421, 90.6165, 145.0583, 215.9372, 302.0713, 402.1070, 513.9517, 646.9866, 784.2588]


# Calculando a média dos tempos de execução
media_tempos = sum(tempos_execucao) / len(tempos_execucao)

print(f"Média dos tempos de execução: {media_tempos:.6f} segundos")