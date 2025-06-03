using Graphs
using GraphPlot
using Colors
using Compose

# Criação de grafo com grau médio aproximado
function create_graph(n::Int, avg_degree::Int)
    graph = [Vector{Int}() for _ in 1:n]  # Lista de adjacência

    # Conecta todos os vértices como uma árvore (conectividade garantida)
    for i in 2:n
        parent = rand(1:i-1)
        push!(graph[i], parent)
        push!(graph[parent], i)
    end

    # Adiciona arestas extras para atingir grau médio
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

# Visualização da BFS camada por camada
function visualize_bfs(graph_list::Vector{Vector{Int}}, start::Int)
    n = length(graph_list)
    g = SimpleGraph(n)

    # Constrói o grafo do pacote Graphs.jl
    for u in 1:n
        for v in graph_list[u]
            if u < v  # evita duplicação de arestas
                add_edge!(g, u, v)
            end
        end
    end

    visited = falses(n)
    visited[start] = true
    frontier = [start]
    level = 0

    while !isempty(frontier)
        println("Camada $level: ", frontier)

        # Define cores: branco = não visitado, cinza = visitado, vermelho = atual
        color_map = [visited[i] ? RGB(0.5, 0.5, 0.5) : RGB(1, 1, 1) for i in 1:n]
        for v in frontier
            color_map[v] = RGB(1.0, 0.0, 0.0)
        end

        # Salva visualização como SVG
        fname = "bfs_nivel_$level.svg"
        draw(SVG(fname, 600px, 600px), gplot(g, nodefillc=color_map))
        println("  → Salvo: $fname")

        # Explora próximo nível
        next_frontier = Int[]
        for u in frontier
            for v in graph_list[u]
                if !visited[v]
                    visited[v] = true
                    push!(next_frontier, v)
                end
            end
        end

        frontier = next_frontier
        level += 1
    end
end
