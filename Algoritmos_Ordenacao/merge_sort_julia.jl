using Base.Threads, Statistics

# Função de mesclagem usando vetor auxiliar
function merge!(A, B, lo, mid, hi)
    i, j, k = lo, mid+1, lo
    while i ≤ mid && j ≤ hi
        if A[i] ≤ A[j]
            B[k] = A[i]
            i += 1
        else
            B[k] = A[j]
            j += 1
        end
        k += 1
    end
    while i ≤ mid
        B[k] = A[i]
        i += 1; k += 1
    end
    while j ≤ hi
        B[k] = A[j]
        j += 1; k += 1
    end
end

# Função recursiva paralela que ordena o vetor A entre os índices lo e hi usando B como auxiliar
function pmergesort!(A, B, lo, hi, thresh=10_000)
    # Se o subvetor for pequeno, ordena sequencialmente
    if hi - lo + 1 ≤ thresh
        sort!(view(A, lo:hi))
        return
    end
    mid = (lo + hi) ÷ 2
    # Ordena a metade esquerda em paralelo
    left_task = @spawn pmergesort!(A, B, lo, mid, thresh)
    # Ordena a metade direita na thread atual
    pmergesort!(A, B, mid+1, hi, thresh)
    # Aguarda a conclusão da tarefa paralela
    fetch(left_task)
    # Mescla os dois subvetores ordenados em B
    merge!(A, B, lo, mid, hi)
    # Copia a porção mesclada de volta para A
    A[lo:hi] .= B[lo:hi]
end

# Função principal que prepara o vetor auxiliar e chama a ordenação paralela
function parallel_mergesort!(A; thresh=10_000)
    B = similar(A)
    pmergesort!(A, B, 1, length(A), thresh)
    return A
end

# Função para executar os testes com diferentes tamanhos
function executar_teste()
    tamanhos = [1_000_000, 5_000_000, 10_000_000, 25_000_000, 50_000_000,
                75_000_000, 100_000_000]
    tempos = Float64[]
    
    for tamanho in tamanhos
        vetor = rand(1:100000, tamanho)
        inicio = time()
        sorted = parallel_mergesort!(vetor)
        tempo_execucao = round(time() - inicio, digits=4)
        push!(tempos, tempo_execucao)
        println("Tamanho: $tamanho - Tempo: $tempo_execucao segundos")
    end
    
    media = round(mean(tempos), digits=4)
    println("\nTempo médio: $media segundos")
end

executar_teste()
