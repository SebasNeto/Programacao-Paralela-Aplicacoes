tempos_execucao = [
    0.0123, 0.1008, 0.3410, 0.8381, 1.6491, 
    3.0936, 5.2485, 7.2844, 11.1128, 14.2354
]


# Calculando a média dos tempos de execução
media_tempos = sum(tempos_execucao) / len(tempos_execucao)

print(f"Média dos tempos de execução: {media_tempos:.4f} segundos")