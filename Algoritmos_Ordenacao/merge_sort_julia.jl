using Base.Threads, Statistics

const BASE_THRESH = 10_000  # Threshold base para paralelismo

# Função otimizada para mesclar dois subarrays ordenados
@inline function merge!(A, B, lo, mid, hi)
    i, j, k = lo, mid + 1, lo
    @inbounds while i ≤ mid && j ≤ hi
        if A[i] ≤ A[j]
            B[k] = A[i]
            i += 1
        else
            B[k] = A[j]
            j += 1
        end
        k += 1
    end
    @inbounds while i ≤ mid
        B[k] = A[i]
        i += 1; k += 1
    end
    @inbounds while j ≤ hi
        B[k] = A[j]
        j += 1; k += 1
    end
end

# Implementação otimizada do Merge Sort paralelo
function pmergesort!(A, B, lo, hi, depth, max_depth, use_AtoB, threshold)
    if hi - lo + 1 ≤ threshold
        sort!(view(A, lo:hi))  # Usa ordenação nativa para subarrays pequenos
        if !use_AtoB
            B[lo:hi] .= A[lo:hi]  # Copia o resultado de volta se necessário
        end
        return
    end

    mid = (lo + hi) ÷ 2

    if depth < max_depth
        left_task = @spawn pmergesort!(A, B, lo, mid, depth + 1, max_depth, !use_AtoB, threshold)
        pmergesort!(A, B, mid + 1, hi, depth + 1, max_depth, !use_AtoB, threshold)
        fetch(left_task)
    else
        pmergesort!(A, B, lo, mid, depth + 1, max_depth, !use_AtoB, threshold)
        pmergesort!(A, B, mid + 1, hi, depth + 1, max_depth, !use_AtoB, threshold)
    end

    if use_AtoB
        merge!(A, B, lo, mid, hi)
    else
        merge!(B, A, lo, mid, hi)
    end
end

# Função principal que define um limiar de paralelismo adaptável
function parallel_mergesort!(A)
    max_threads = Threads.nthreads()
    max_depth = Int(log2(max_threads))  # Define a profundidade máxima com base no número de threads
    dynamic_thresh = max(BASE_THRESH, length(A) ÷ (2 * max_threads))  # Ajuste dinâmico do THRESHOLD

    B = similar(A)
    pmergesort!(A, B, 1, length(A), 0, max_depth, true, dynamic_thresh)
    return A
end

# Função de teste para medir o desempenho
function executar_teste()
    tamanhos = [1_000_000, 5_000_000, 10_000_000, 25_000_000, 50_000_000,
                75_000_000, 100_000_000, 250_000_000, 500_000_000]
    tempos = Float64[]
    
    println("Número de threads: ", Threads.nthreads())
    println("Profundidade máxima de paralelismo: ", Int(log2(Threads.nthreads())))
    println("Threshold dinâmico ajustado para tamanho do vetor\n")

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


