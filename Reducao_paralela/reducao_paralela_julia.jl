using Base.Threads

const NUM_ITERACOES = 10
const tamanhos_array = [10_000_000, 20_000_000, 30_000_000, 40_000_000, 50_000_000,
                        60_000_000, 70_000_000, 80_000_000, 90_000_000, 100_000_000]

global soma_dummy = 0

# Função otimizada para redução paralela com divisão de blocos, @inbounds e @simd
function soma_paralela_rapida(arr::Vector{Int})
    num_threads = nthreads()
    somas_parciais = zeros(Int, num_threads)
    n = length(arr)
    bloco = div(n, num_threads)
    resto = mod(n, num_threads)
    @threads for id_thread in 1:num_threads
        # Calcula os índices de início e fim para cada thread
        indice_inicio = (id_thread - 1) * bloco + min(id_thread - 1, resto) + 1
        indice_fim = id_thread * bloco + min(id_thread, resto)
        soma_local = 0
        @inbounds @simd for i in indice_inicio:indice_fim
            soma_local += arr[i]
        end
        somas_parciais[id_thread] = soma_local
    end
    return sum(somas_parciais)
end

function executar_benchmark()
    tempo_total = 0.0
    for tamanho in tamanhos_array
        # Cria um vetor com inteiros aleatórios entre 0 e 9
        arr = rand(0:9, tamanho)
        soma_tempos = 0.0
        for iteracao in 1:NUM_ITERACOES
            tempo_inicio = time_ns()
            s = soma_paralela_rapida(arr)
            tempo_fim = time_ns()
            tempo_decorrido = (tempo_fim - tempo_inicio) / 1e9  # converte nanosegundos para segundos
            soma_tempos += tempo_decorrido
            global soma_dummy += s
        end
        tempo_medio = soma_tempos / NUM_ITERACOES
        tempo_total += tempo_medio
        println("Tamanho do vetor: $tamanho -> Tempo médio: $(tempo_medio) segundos")
    end
    tempo_medio_total = tempo_total / length(tamanhos_array)
    println("\nTempo médio geral: $(tempo_medio_total) segundos")
    println("Soma total (dummy): $soma_dummy")
end

executar_benchmark()
