using Base.Threads, Statistics

const BASE_THRESH = 10_000  # Tamanho mínimo para paralelismo

# Função otimizada para mesclar dois subarrays ordenados
@inline function merge!(A, B, lo, mid, hi)
    i, j, k = lo, mid+1, lo
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
    # Copia apenas a parte mesclada de volta para A (evita cópias desnecessárias)
    unsafe_copyto!(pointer(A, lo), pointer(B, lo), hi - lo + 1)
end

# Implementação otimizada do Merge Sort paralelo
function pmergesort!(A, B, lo, hi, thresh)
    if hi - lo + 1 ≤ thresh
        sort!(view(A, lo:hi))  # Usa ordenação nativa para subarrays pequenos
        return
    end

    mid = (lo + hi) ÷ 2
    # Paraleliza apenas se o tamanho for suficientemente grande
    if hi - lo > thresh * 2
        left_task = @spawn pmergesort!(A, B, lo, mid, thresh)
        pmergesort!(A, B, mid+1, hi, thresh)
        fetch(left_task)
    else
        pmergesort!(A, B, lo, mid, thresh)
        pmergesort!(A, B, mid+1, hi, thresh)
    end
    merge!(A, B, lo, mid, hi)
end

# Função principal que define um limiar de paralelismo adaptável
function parallel_mergesort!(A; thresh=BASE_THRESH * Threads.nthreads())
    B = similar(A)
    pmergesort!(A, B, 1, length(A), thresh)
    return A
end

# Função de teste para medir o desempenho
function executar_teste()
    tamanhos = [1_000_000, 5_000_000, 10_000_000, 25_000_000, 50_000_000,
                75_000_000, 100_000_000, 250_000_000, 500_000_000]
    tempos = Float64[]
    
    println("Número de threads: ", Threads.nthreads())
    println("Threshold ajustado: ", BASE_THRESH * Threads.nthreads(), "\n")

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
