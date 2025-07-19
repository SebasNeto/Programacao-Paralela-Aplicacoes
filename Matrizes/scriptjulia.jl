using LinearAlgebra, BenchmarkTools, Printf, Distributed, Statistics

# Função otimizada usando BLAS para multiplicação de matrizes
function multiplicacao_matriz!(C, A, B)
    mul!(C, A, B)  # Usa BLAS otimizado
end

# Configuração de threads
println("Número de threads disponíveis: ", Threads.nthreads())

# Tamanhos das matrizes para teste
tamanhos = 1000:1000:10000
tempos = Float64[]

for N in tamanhos
    println("Processando matriz $N x $N...")

    # Gera matrizes aleatórias
    A = rand(N, N)
    B = rand(N, N)
    C = zeros(N, N)  # Matriz para armazenar resultado

   
    tempo_exec = @belapsed multiplicacao_matriz!($(Ref(C))[], $(Ref(A))[], $(Ref(B))[])
    
    push!(tempos, tempo_exec)
    @printf("Tempo para %d x %d: %.4f segundos\n", N, N, tempo_exec)
end


@printf("\nMédia do tempo de execução: %.4f segundos\n", mean(tempos))
