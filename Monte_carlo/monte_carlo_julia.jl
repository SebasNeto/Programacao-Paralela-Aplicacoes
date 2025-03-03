using Random, Statistics, Base.Threads

const NUM_ITER = 10
const NUM_SAMPLES = 100_000_000  # Número total de amostras por iteração

# Função que processa um número específico de amostras usando um gerador RNG fornecido
function monte_carlo_pi_thread(samples::Int, rng::AbstractRNG)
    count = 0
    for _ in 1:samples
        x = rand(rng) * 2.0 - 1.0
        y = rand(rng) * 2.0 - 1.0
        if x*x + y*y <= 1.0
            count += 1
        end
    end
    return count
end

# Função paralela que distribui as amostras entre as threads e retorna a estimativa de π
function monte_carlo_pi_parallel(samples_total::Int)
    nthreads = Threads.nthreads()
    samples_per_thread = div(samples_total, nthreads)
    remainder = samples_total % nthreads
    counts = zeros(Int, nthreads)
    
    @threads for tid in 1:nthreads
        # Cada thread processa um número de amostras ajustado para distribuir o resto
        samples = samples_per_thread + (tid <= remainder ? 1 : 0)
        # Inicializa um gerador de números aleatórios específico para a thread
        rng = MersenneTwister(Threads.threadid() + 1234 + tid)
        counts[tid] = monte_carlo_pi_thread(samples, rng)
    end
    total_count = sum(counts)
    return 4.0 * total_count / samples_total
end

# Função principal que executa várias iterações, mede o tempo e calcula a média
function main()
    times = Float64[]
    for iter in 1:NUM_ITER
        t_start = time()
        pi_est = monte_carlo_pi_parallel(NUM_SAMPLES)
        t_end = time()
        elapsed = t_end - t_start
        push!(times, elapsed)
        println("Iteração $iter: pi = $pi_est, tempo = $(elapsed) segundos")
    end
    println("Tempo médio: $(mean(times)) segundos")
end

main()
