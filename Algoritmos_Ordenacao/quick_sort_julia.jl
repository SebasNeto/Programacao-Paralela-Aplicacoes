using Base.Threads, Statistics
#correct
const MIN_SIZE = 100_000  # Ajuste baseado no desempenho prático

@inline function choose_pivot!(vetor, baixo, alto)
    mid = baixo + (alto - baixo) ÷ 2
    if vetor[baixo] > vetor[mid]
        vetor[baixo], vetor[mid] = vetor[mid], vetor[baixo]
    end
    if vetor[baixo] > vetor[alto]
        vetor[baixo], vetor[alto] = vetor[alto], vetor[baixo]
    end
    if vetor[mid] > vetor[alto]
        vetor[mid], vetor[alto] = vetor[alto], vetor[mid]
    end
    vetor[mid], vetor[alto] = vetor[alto], vetor[mid]
    return vetor[alto]
end

@inline function particionar!(vetor, baixo, alto)
    pivo = choose_pivot!(vetor, baixo, alto)
    i, j = baixo, alto - 1
    while true
        while vetor[i] < pivo
            i += 1
        end
        while j > baixo && vetor[j] > pivo
            j -= 1
        end
        if i >= j
            break
        end
        vetor[i], vetor[j] = vetor[j], vetor[i]
        i += 1
        j -= 1
    end
    vetor[i], vetor[alto] = vetor[alto], vetor[i]
    return i
end

function quicksort_sequencial!(vetor, baixo, alto)
    while baixo < alto
        pi = particionar!(vetor, baixo, alto)
        if pi - baixo < alto - pi
            quicksort_sequencial!(vetor, baixo, pi - 1)
            baixo = pi + 1
        else
            quicksort_sequencial!(vetor, pi + 1, alto)
            alto = pi - 1
        end
    end
end

function quicksort_parallel!(vetor, baixo, alto, depth=0, max_depth=Threads.nthreads() * 2)
    if baixo < alto
        if (alto - baixo) < MIN_SIZE || depth >= max_depth
            quicksort_sequencial!(vetor, baixo, alto)
            return
        end

        pi = particionar!(vetor, baixo, alto)

        @threads for i in 1:2
            if i == 1
                quicksort_parallel!(vetor, baixo, pi - 1, depth + 1, max_depth)
            else
                quicksort_parallel!(vetor, pi + 1, alto, depth + 1, max_depth)
            end
        end
    end
end

function executar_teste()
    tamanhos = [1_000_000, 5_000_000, 10_000_000, 25_000_000, 50_000_000,
                75_000_000, 100_000_000, 250_000_000, 500_000_000]
    tempos = Float64[]

    for tamanho in tamanhos
        vetor = rand(1:100_000, tamanho)
        inicio = time()
        quicksort_parallel!(vetor, 1, length(vetor))
        tempo_execucao = round(time() - inicio, digits=4)
        push!(tempos, tempo_execucao)
        println("Tamanho: $tamanho - Tempo: $tempo_execucao segundos")
    end

    media_tempo = round(mean(tempos), digits=4)
    println("\nMédia geral dos tempos de execução: $media_tempo segundos")
end

executar_teste()
