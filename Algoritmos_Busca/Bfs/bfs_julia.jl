using Random
using Base.Threads
using DataStructures

# Criação otimizada do grafo
function create_graph(n::Int, avg_degree::Int)
    graph = [Vector{Int}() for _ in 1:n]
    for i in 2:n
        parent = rand(1:i-1)
        push!(graph[i], parent)
        push!(graph[parent], i)
    end
    total_edges = div(avg_degree * n, 2)
    extra_edges = total_edges - (n - 1)
    for _ in 1:extra_edges
        u, v = rand(1:n), rand(1:n)
        if u != v && v ∉ graph[u]
            push!(graph[u], v)
            push!(graph[v], u)
        end
    end
    return graph
end

# BFS Paralela com controle de concorrência
function parallel_bfs_optimized(graph::Vector{Vector{Int}}, start::Int)
    n = length(graph)
    distances = fill(-1, n)
    distances[start] = 0
    frontier = [start]
    level = 0

    while !isempty(frontier)
        # Cria buffers locais para cada thread
        local_next = [Vector{Int}() for _ in 1:Threads.nthreads()]
        frontier_copy = copy(frontier)

        Threads.@threads for i in 1:length(frontier_copy)
            u = frontier_copy[i]
            @inbounds for v in graph[u]
                # Checagem sem lock; pode ocorrer condição de corrida,
                # mas, se ambos escreverem o mesmo valor (level+1), o impacto é apenas
                # uma possível inserção duplicada na próxima fronteira.
                if distances[v] == -1
                    distances[v] = level + 1
                    push!(local_next[Threads.threadid()], v)
                end
            end
        end

        # Combina os buffers locais na próxima fronteira
        next_frontier = reduce(vcat, local_next)
        frontier = next_frontier
        level += 1
    end

    return distances
end


function main()
    sizes = [500000, 600000, 700000, 800000, 900000, 1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 7000000]
    avg_degree = 10
    total_time = 0.0

    for s in sizes
        println("Tamanho do grafo: $s vértices")
        graph = create_graph(s, avg_degree)
        t_start = time()
        distances = parallel_bfs_optimized(graph, 1)  
        t_end = time()
        t = t_end - t_start
        total_time += t
        println("  Tempo: $t segundos\n")
    end

    average_time = total_time / length(sizes)
    println("Tempo médio (BFS com Threads em Julia): $average_time segundos")
end

main()
