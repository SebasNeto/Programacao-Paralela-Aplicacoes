using Random
using Base.Threads
using DataStructures

# Criação otimizada do grafo
function criarGrafo(n::Int, avg_degree::Int)
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

# BFS Paralela 
function paraleloBfs(graph::Vector{Vector{Int}}, start::Int)
    n = length(graph)
    distances = fill(-1, n)
    distances[start] = 0
    frontier = [start]
    level = 0

    while !isempty(frontier)
        next_frontier = Vector{Int}()
        frontier_copy = copy(frontier)

        #vetor para armazenar resultados de cada thread
        local_next = [Vector{Int}() for _ in 1:Threads.nthreads()]

        Threads.@threads for u in frontier_copy
            tid = Threads.threadid()
            for v in graph[u]
                if distances[v] == -1
                    distances[v] = level + 1
                    push!(local_next[tid], v)
                end
            end
        end

        # Combina os resultados de todas as threads
        for vec in local_next
            append!(next_frontier, vec)
        end

        frontier = next_frontier
        level += 1
    end

    return distances
end

function main()
    sizes = [1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 700000, 8000000, 9000000, 10000000]
    avg_degree = 10
    total_time = 0.0

    for s in sizes
        println("Tamanho do grafo: $s vértices")
        graph = criarGrafo(s, avg_degree)
        t_start = time()
        distances = paraleloBfs(graph, 1)  
        t_end = time()
        t = t_end - t_start
        total_time += t
        println("  Tempo: $t segundos\n")
    end

    average_time = total_time / length(sizes)
    println("Tempo médio (BFS com Threads em Julia): $average_time segundos")
end

main()