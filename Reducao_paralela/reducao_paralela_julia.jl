using Base.Threads
#correct
const NUM_ITER = 10
const array_sizes = [10_000_000, 20_000_000, 30_000_000, 40_000_000, 50_000_000,
                     60_000_000, 70_000_000, 80_000_000, 90_000_000, 100_000_000]

global total_dummy = 0

# Função otimizada para redução paralela com chunking, @inbounds e @simd
function fast_parallel_sum(arr::Vector{Int})
    nt = nthreads()
    partials = zeros(Int, nt)
    n = length(arr)
    chunk = div(n, nt)
    remainder = mod(n, nt)
    @threads for tid in 1:nt
        # Calcula os índices de início e fim para cada thread
        start_index = (tid - 1) * chunk + min(tid - 1, remainder) + 1
        end_index = tid * chunk + min(tid, remainder)
        local_sum = 0
        @inbounds @simd for i in start_index:end_index
            local_sum += arr[i]
        end
        partials[tid] = local_sum
    end
    return sum(partials)
end

function run_benchmark()
    total_time = 0.0
    for n in array_sizes
        # Cria um vetor com inteiros aleatórios entre 0 e 9
        arr = rand(0:9, n)
        time_sum = 0.0
        for iter in 1:NUM_ITER
            t_start = time_ns()
            s = fast_parallel_sum(arr)
            t_end = time_ns()
            elapsed = (t_end - t_start) / 1e9  # converte nanosegundos para segundos
            time_sum += elapsed
            global total_dummy += s
        end
        avg_time = time_sum / NUM_ITER
        total_time += avg_time
        println("Tamanho do vetor: $n -> Tempo médio: $(avg_time) segundos")
    end
    avg_total_time = total_time / length(array_sizes)
    println("\nTempo médio geral: $(avg_total_time) segundos")
    println("Soma total (dummy): $total_dummy")
end

run_benchmark()
