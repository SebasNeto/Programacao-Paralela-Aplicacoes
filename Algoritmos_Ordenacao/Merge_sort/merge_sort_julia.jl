using Base.Threads, Statistics

const LIMIAR_BASE = 10_000  # Limiar base para paralelismo

# Função otimizada para mesclar dois subarrays ordenados
@inline function mesclar!(A, B, inicio, meio, fim)
    i, j, k = inicio, meio + 1, inicio
    @inbounds while i ≤ meio && j ≤ fim
        if A[i] ≤ A[j]
            B[k] = A[i]
            i += 1
        else
            B[k] = A[j]
            j += 1
        end
        k += 1
    end
    @inbounds while i ≤ meio
        B[k] = A[i]
        i += 1; k += 1
    end
    @inbounds while j ≤ fim
        B[k] = A[j]
        j += 1; k += 1
    end
end

# Implementação otimizada do Merge Sort paralelo
function merge_sort_paralelo!(A, B, inicio, fim, profundidade, max_profundidade, usar_A_para_B, limiar)
    if fim - inicio + 1 ≤ limiar
        sort!(view(A, inicio:fim))  # Usa ordenação nativa para subarrays pequenos
        if !usar_A_para_B
            B[inicio:fim] .= A[inicio:fim]  # Copia o resultado de volta se necessário
        end
        return
    end

    meio = (inicio + fim) ÷ 2

    if profundidade < max_profundidade
        tarefa_esquerda = @spawn merge_sort_paralelo!(A, B, inicio, meio, profundidade + 1, max_profundidade, !usar_A_para_B, limiar)
        merge_sort_paralelo!(A, B, meio + 1, fim, profundidade + 1, max_profundidade, !usar_A_para_B, limiar)
        fetch(tarefa_esquerda)
    else
        merge_sort_paralelo!(A, B, inicio, meio, profundidade + 1, max_profundidade, !usar_A_para_B, limiar)
        merge_sort_paralelo!(A, B, meio + 1, fim, profundidade + 1, max_profundidade, !usar_A_para_B, limiar)
    end

    if usar_A_para_B
        mesclar!(A, B, inicio, meio, fim)
    else
        mesclar!(B, A, inicio, meio, fim)
    end
end

# Função principal que define um limiar de paralelismo adaptável
function ordenacao_merge_paralelo!(A)
    max_threads = Threads.nthreads()
    max_profundidade = Int(log2(max_threads))  # Define a profundidade máxima com base no número de threads
    limiar_dinamico = max(LIMIAR_BASE, length(A) ÷ (2 * max_threads))  # Ajuste dinâmico do limiar

    B = similar(A)
    merge_sort_paralelo!(A, B, 1, length(A), 0, max_profundidade, true, limiar_dinamico)
    return A
end

# Função de teste para medir o desempenho
function executar_teste()
    tamanhos = [10_000_000, 20_000_000, 30_000_000, 40_000_000, 50_000_000,
                60_000_000, 70_000_000, 80_000_000, 90_000_000, 100_000_000]
    tempos = Float64[]
    
    println("Número de threads: ", Threads.nthreads())
    println("Profundidade máxima de paralelismo: ", Int(log2(Threads.nthreads())))
    println("Limiar dinâmico ajustado para tamanho do vetor\n")

    for tamanho in tamanhos
        vetor = rand(1:100000, tamanho)
        inicio = time()
        ordenado = ordenacao_merge_paralelo!(vetor)
        tempo_execucao = round(time() - inicio, digits=4)
        push!(tempos, tempo_execucao)
        println("Tamanho: $tamanho - Tempo: $tempo_execucao segundos")
    end
    
    media = round(mean(tempos), digits=4)
    println("\nTempo médio: $media segundos")
end

executar_teste()


