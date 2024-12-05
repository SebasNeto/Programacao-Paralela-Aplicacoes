using Distributed
using LinearAlgebra  # Para multiplicação de matrizes

function parallel_matrix_multiplication(n::Int)
    # Inicializando as matrizes
    a = fill(1.0, n, n)  # Matriz A preenchida com 1.0
    b = fill(2.0, n, n)  # Matriz B preenchida com 2.0

    # Medindo o tempo de execução
    start_time = time()
    c = a * b  # Multiplicação otimizada (nativa de Julia)
    end_time = time()

    # Calculando o tempo decorrido
    elapsed_time = end_time - start_time
    println("Tempo de execução: $elapsed_time segundos")

    # Retorno da matriz resultado
    return c
end

# Parâmetro: tamanho da matriz
n = 1000  # Ajuste o tamanho conforme necessário
println("Iniciando a multiplicação de matrizes de tamanho $n x $n...")
c = parallel_matrix_multiplication(n)
println("Multiplicação concluída!")
