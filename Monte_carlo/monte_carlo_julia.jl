using Random, Statistics, Base.Threads

const NUM_ITERACOES = 1

# Lista de tamanhos de amostras
tamanhos_amostras = [
    10_000_000, 20_000_000, 30_000_000, 40_000_000, 50_000_000,
    60_000_000, 70_000_000, 80_000_000, 90_000_000, 100_000_000
]

function monteCarlo(amostras::Int, rng::AbstractRNG)
    contador = 0
    for _ in 1:amostras
        x = rand(rng) * 2.0 - 1.0
        y = rand(rng) * 2.0 - 1.0
        if x*x + y*y <= 1.0
            contador += 1
        end
    end
    return contador
end

# Função paralela que distribui as amostras entre as threads e retorna a estimativa de π
function monteCarloParalelo(total_amostras::Int)
    num_threads = Threads.nthreads()
    amostras_por_thread = div(total_amostras, num_threads)
    resto = total_amostras % num_threads
    contadores = zeros(Int, num_threads)
    
    @threads for id_thread in 1:num_threads
        # Cada thread processa um número de amostras ajustado para distribuir o resto
        amostras = amostras_por_thread + (id_thread <= resto ? 1 : 0)
        # Inicializa um gerador de números aleatórios específico para a thread
        rng = MersenneTwister(Threads.threadid() + 1234 + id_thread)
        contadores[id_thread] = monteCarlo(amostras, rng)
    end
    total_contador = sum(contadores)
    return 4.0 * total_contador / total_amostras
end

function principal()
    for num_amostras in tamanhos_amostras
        tempos = Float64[]
        println("\nTamanho da amostra: $num_amostras")
        for iteracao in 1:NUM_ITERACOES
            inicio_tempo = time()
            pi_estimado = monteCarloParalelo(num_amostras)
            fim_tempo = time()
            tempo_decorrido = fim_tempo - inicio_tempo
            push!(tempos, tempo_decorrido)
            println("Iteração $iteracao: pi = $pi_estimado, tempo = $(tempo_decorrido) segundos")
        end
        println("Tempo médio para amostra $num_amostras: $(mean(tempos)) segundos")
    end
end

principal()




