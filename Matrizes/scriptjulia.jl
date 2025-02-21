using Distributed
using LinearAlgebra

# Adiciona workers para paralelismo (usando todos os núcleos disponíveis)
addprocs(Sys.CPU_THREADS)

# Carrega o módulo LinearAlgebra em todos os workers
@everywhere using LinearAlgebra

# Função para multiplicação de matrizes com transposição e paralelismo
function multiplicar_matrizes_paralelo(A, B)
    n = size(A, 1)
    C = zeros(n, n)  # Matriz resultado

    # Transposição de B
    B_transposta = transpose(B)

    # Paraleliza a multiplicação usando @distributed
    @sync @distributed for i in 1:n
        for j in 1:n
            C[i, j] = dot(A[i, :], B_transposta[j, :])
        end
    end

    return C
end

# Função principal
function main()
    tamanhos = [1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000]
    tempo_total = 0.0

    for n in tamanhos
        # Gera matrizes aleatórias
        A = rand(n, n)
        B = rand(n, n)

        # Medição do tempo de execução
        tempo = @elapsed multiplicar_matrizes_paralelo(A, B)

        println("Tamanho da matriz: $n x $n, Tempo de execução: $tempo segundos")
        tempo_total += tempo
    end

    tempo_medio = tempo_total / length(tamanhos)
    println("\nTempo médio de execução: $tempo_medio segundos")
end

# Executa o programa
main()