using Random, Distributed

const tamanhos = [
    1_000_000, 5_000_000, 10_000_000, 25_000_000, 50_000_000,
    75_000_000, 100_000_000, 250_000_000, 500_000_000
]
const NUM_BUSCAS = 1000

@inline function busca_binaria(arr::Vector{Int}, x::Int)
    esquerda = 1
    direita = length(arr)
    while esquerda <= direita
        meio = (esquerda + direita) >>> 1
        @inbounds begin
            valor = arr[meio]
            if valor == x
                return meio
            elseif valor < x
                esquerda = meio + 1
            else
                direita = meio - 1
            end
        end
    end
    return -1
end

function busca_paralela(arr::Vector{Int}, buscas::Vector{Int})
    resultados = Vector{Int}(undef, length(buscas))
    n = length(buscas)
    chunk = cld(n, Threads.nthreads())
    Threads.@threads for t in 1:Threads.nthreads()
        inicio = (t - 1) * chunk + 1
        fim = min(t * chunk, n)
        for i in inicio:fim
            resultados[i] = busca_binaria(arr, buscas[i])
        end
    end
    return resultados
end

function executar_testes()
    tempos = Float64[]
    
    for tamanho in tamanhos
        arr = collect(1:2:(2 * tamanho))
        buscas = rand(1:2 * tamanho, NUM_BUSCAS)
        
        tempo_paralelo = @elapsed busca_paralela(arr, buscas)
        push!(tempos, tempo_paralelo)
        println("Tamanho: $tamanho - Tempo da busca binária paralela: $tempo_paralelo segundos")
    end
    
    media_tempo = sum(tempos) / length(tempos)
    println("Média do tempo de execução paralela: $media_tempo segundos")
end

executar_testes()
