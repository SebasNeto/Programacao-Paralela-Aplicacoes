using Base.Threads, Statistics

const MIN_SIZE = 100_000  # Aumentado para reduzir a criação excessiva de tarefas

function particionar!(vetor, baixo, alto)
    pivo = vetor[alto]
    i = baixo - 1
    for j in baixo:(alto - 1)
        if vetor[j] < pivo
            i += 1
            vetor[i], vetor[j] = vetor[j], vetor[i]
        end
    end
    vetor[i + 1], vetor[alto] = vetor[alto], vetor[i + 1]
    return i + 1
end

# Versão sequencial para partições pequenas ou quando a profundidade máxima for atingida
function quicksort_sequencial!(vetor, baixo, alto)
    if baixo < alto
        pi = particionar!(vetor, baixo, alto)
        quicksort_sequencial!(vetor, baixo, pi - 1)
        quicksort_sequencial!(vetor, pi + 1, alto)
    end
end

function quicksort_parallel!(vetor, baixo, alto, depth=0, max_depth=Threads.nthreads())
    if baixo < alto
        # Se a partição for pequena ou se atingiu a profundidade máxima, use a versão sequencial
        if (alto - baixo) < MIN_SIZE || depth >= max_depth
            quicksort_sequencial!(vetor, baixo, alto)
            return
        end

        pi = particionar!(vetor, baixo, alto)
        t1 = @spawn quicksort_parallel!(vetor, baixo, pi - 1, depth + 1, max_depth)
        t2 = @spawn quicksort_parallel!(vetor, pi + 1, alto, depth + 1, max_depth)
        fetch(t1)
        fetch(t2)
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
