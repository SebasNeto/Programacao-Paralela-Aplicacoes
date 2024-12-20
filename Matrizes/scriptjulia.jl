using Distributed
using LinearAlgebra  # Para multiplicação de matrizes

function parallel_matrix_multiplication(n::Int)
    # Inicializando as matrizes
    a = fill(1.0, n, n)
    b = fill(2.0, n, n)

    start_time = time()
    c = a * b  # Multiplicação de matrizes
    end_time = time()

    # Calculando o tempo decorrido
    elapsed_time = end_time - start_time
    println("Tamanho da matriz: $n x $n, Tempo de execução: $elapsed_time segundos")

    # Retorno do tempo para salvar no arquivo
    return elapsed_time
end

# Intervalo de tamanhos de matrizes para teste
function test_matrix_sizes(start_size::Int, end_size::Int, step::Int)
    # Abrindo arquivo para escrita
    open("resultados.csv", "w") do file
        # Escrevendo cabeçalho
        println(file, "Tamanho,Tempo (segundos)")
        
        # Loop para testar diferentes tamanhos de matrizes
        for n in start_size:step:end_size
            println("\nIniciando a multiplicação de matrizes de tamanho $n x $n...")
            elapsed_time = parallel_matrix_multiplication(n)

            # Salvando resultado no arquivo
            println(file, "$n,$elapsed_time")
        end
    end
end

# Configuração dos testes
start_size = 1000  # Tamanho inicial
end_size = 10000   # Tamanho final
step = 1000        # Incremento do tamanho

test_matrix_sizes(start_size, end_size, step)

