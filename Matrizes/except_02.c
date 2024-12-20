//arquivo julia inicial

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

//próximo arquivo openmp inicial

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

// gcc -DDEBUG scriptmp.c -o testemp -fopenmpcl

int main() {
    int N = 1000;
    int **A, **B, **C;

    // Alocação dinâmica das matrizes (mesmo que no código sequencial)
    // [Alocação e inicialização das matrizes A, B e C aqui]

    A = (int **)malloc(N * sizeof(int *));
    B = (int **)malloc(N * sizeof(int *));
    C = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        A[i] = (int *)malloc(N * sizeof(int));
        B[i] = (int *)malloc(N * sizeof(int));
        C[i] = (int *)malloc(N * sizeof(int));
    }

    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
        }
    }

    double start = omp_get_wtime();

    #pragma omp parallel for shared(A, B, C, N) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }

    double end = omp_get_wtime();
    printf("Tempo de execução com OpenMP: %f segundos\n", end - start);

    // Liberação da memória alocada
    for (int i = 0; i < N; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);

    return 0;
}

//versao openmp automatizado

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

void multiplicarMatrizesComOpenMP(int N) {
    int **A, **B, **C;

    // Alocação dinâmica das matrizes
    A = (int **)malloc(N * sizeof(int *));
    B = (int **)malloc(N * sizeof(int *));
    C = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        A[i] = (int *)malloc(N * sizeof(int));
        B[i] = (int *)malloc(N * sizeof(int));
        C[i] = (int *)malloc(N * sizeof(int));
    }

    // Inicialização das matrizes
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
        }
    }

    // Multiplicação com OpenMP
    double start = omp_get_wtime();
    #pragma omp parallel for shared(A, B, C, N) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    double end = omp_get_wtime();

    // Exibição do tempo de execução
    printf("Tamanho: %d, Tempo de execução com OpenMP: %f segundos\n", N, end - start);

    // Salvando os resultados em um arquivo CSV
    FILE *file = fopen("resultados_omp.csv", "a");
    if (file != NULL) {
        fprintf(file, "%d,%f\n", N, end - start);
        fclose(file);
    }

    // Liberação da memória alocada
    for (int i = 0; i < N; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);
}

int main() {
    int start_size = 1000; // Tamanho inicial
    int end_size = 10000;  // Tamanho final
    int step = 1000;       // Incremento do tamanho

    // Cabeçalho do arquivo CSV
    FILE *file = fopen("resultados_omp.csv", "w");
    if (file != NULL) {
        fprintf(file, "Tamanho,Tempo (segundos)\n");
        fclose(file);
    }

    // Loop para testar tamanhos variados de matrizes
    for (int N = start_size; N <= end_size; N += step) {
        multiplicarMatrizesComOpenMP(N);
    }

    return 0;
}
