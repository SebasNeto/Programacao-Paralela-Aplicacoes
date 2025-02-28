using Base.Threads, Distributed, LoopVectorization, Statistics


# Mesclar duas partes do vetor
function mesclar!(vetor, esq, meio, dir)
    temp = copy(vetor[esq:dir])
    i, j, k = 1, meio - esq + 2, esq

    for _ in 1:(dir - esq + 1)
        if i <= meio - esq + 1 && (j > length(temp) || temp[i] <= temp[j])
            vetor[k] = temp[i]
            i += 1
        else
            vetor[k] = temp[j]
            j += 1
        end
        k += 1
    end
end

# Função recursiva paralelizada
function merge_sort_parallel!(vetor, esq, dir, profundidade=0)
    if esq < dir
        meio = esq + (dir - esq) ÷ 2

        if profundidade < Threads.nthreads()  # Evita criar threads em excesso
            tarefa_esq = Threads.@spawn merge_sort_parallel!(vetor, esq, meio, profundidade + 1)
            tarefa_dir = Threads.@spawn merge_sort_parallel!(vetor, meio + 1, dir, profundidade + 1)

            wait(tarefa_esq)
            wait(tarefa_dir)
        else
            merge_sort_parallel!(vetor, esq, meio, profundidade + 1)
            merge_sort_parallel!(vetor, meio + 1, dir, profundidade + 1)
        end

        mesclar!(vetor, esq, meio, dir)
    end
end

# Função para executar testes com diferentes tamanhos e calcular média
function executar_teste()
    tamanhos = [10000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000, 100000]
    tempos = Float64[]

    for tamanho in tamanhos
        vetor = rand(1:100000, tamanho)
        inicio = time()
        merge_sort_parallel!(vetor, 1, length(vetor))
        tempo_execucao = round(time() - inicio, digits=4)
        push!(tempos, tempo_execucao)
        println("Tamanho: $tamanho - Tempo: $tempo_execucao segundos")
    end

    media_tempo = round(mean(tempos), digits=4)
    println("\nMédia geral dos tempos de execução: $media_tempo segundos")
end

# Executar o teste
executar_teste()
