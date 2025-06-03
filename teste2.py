import matplotlib.pyplot as plt
import random
import time

def busca_binaria_visual(array, alvo, linha, ax):
    esquerda, direita = 0, len(array) - 1
    while esquerda <= direita:
        meio = (esquerda + direita) // 2
        cores = ['white'] * len(array)
        for i in range(esquerda, direita + 1):
            cores[i] = 'lightblue'
        cores[meio] = 'red'

        if array[meio] == alvo:
            cores[meio] = 'green'
            ax.clear()
            ax.bar(range(len(array)), array, color=cores)
            ax.set_title(f"Busca {linha}: {alvo} encontrado")
            plt.pause(1)
            return
        elif array[meio] < alvo:
            esquerda = meio + 1
        else:
            direita = meio - 1

        ax.clear()
        ax.bar(range(len(array)), array, color=cores)
        ax.set_title(f"Busca {linha}: buscando {alvo}")
        plt.pause(0.6)

    ax.clear()
    ax.bar(range(len(array)), array, color=['gray'] * len(array))
    ax.set_title(f"Busca {linha}: {alvo} não encontrado")
    plt.pause(1)

def busca_binaria_visual_multipla():
    array = list(range(0, 100, 2))  # Array ordenado
    buscas = random.sample(range(0, 100), 4)  # 4 alvos
    fig, axs = plt.subplots(4, 1, figsize=(10, 6))
    plt.ion()

    for i, alvo in enumerate(buscas):
        busca_binaria_visual(array, alvo, i + 1, axs[i])

    plt.ioff()
    plt.show()

busca_binaria_visual_multipla()
