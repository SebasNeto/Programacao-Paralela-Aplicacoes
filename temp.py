tempos_execucao = [0.2702, 1.0467, 2.4632, 8.3562, 13.2729, 20.6622, 40.5295, 52.8999, 68.9883, 74.4239]


# Calculando a média dos tempos de execução
media_tempos = sum(tempos_execucao) / len(tempos_execucao)

print(f"Média dos tempos de execução: {media_tempos:.6f} segundos")