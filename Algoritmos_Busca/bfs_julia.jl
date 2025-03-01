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
function parallel_bfs(graph::Vector{Vector{Int}}, start::Int)
    n = length(graph)
    distances = fill(-1, n)  # Vetor normal para armazenar distâncias
    distances[start] = 0
    
    frontier = Deque{Int}()  # Estrutura mais eficiente para BFS
    push!(frontier, start)

    level = 0
    lock = Threads.SpinLock()  # Lock para proteger a fronteira

    while !isempty(frontier)
        next_frontier = Deque{Int}()

        # Criar uma cópia segura da fronteira para evitar concorrência
        frontier_copy = collect(frontier)

        @threads for i in 1:length(frontier_copy)
            u = frontier_copy[i]  # Pegamos um nó

            for v in graph[u]
                if distances[v] == -1  # Se ainda não foi visitado
                    distances[v] = level + 1
                    
                    # Protege o acesso a next_frontier
                    Threads.lock(lock)
                    push!(next_frontier, v)
                    Threads.unlock(lock)
                end
            end
        end
        
        # Protege o acesso à fronteira principal
        Threads.lock(lock)
        frontier = next_frontier
        Threads.unlock(lock)

        level += 1
    end
    return distances
end

function main()
    sizes = [100000, 500000, 1000000, 25000000]
    avg_degree = 10
    total_time = 0.0

    for s in sizes
        println("Tamanho do grafo: $s vértices")
        graph = create_graph(s, avg_degree)
        t_start = time()
        distances = parallel_bfs(graph, 1)  
        t_end = time()
        t = t_end - t_start
        total_time += t
        println("  Tempo: $t segundos\n")
    end

    average_time = total_time / length(sizes)
    println("Tempo médio (BFS com Threads em Julia): $average_time segundos")
end

main()
