#include <Halide.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

using namespace Halide;
using namespace std;
using namespace std::chrono;

const int NUM_BUSCAS = 100000;
const int NUM_TAMANHOS = 10;
const int tamanhos[] = { 1000000, 5000000, 10000000, 25000000, 50000000,
                       75000000, 100000000, 250000000, 500000000, 1000000000 };

// Implementação da busca binária em Halide
Expr busca_binaria_halide(Func& arr, Expr tamanho, Expr valor) {
    Expr esquerda = 0;
    Expr direita = tamanho - 1;
    Expr resultado = -1;  // -1 indica não encontrado

    // Número máximo de iterações (log2(1e9) ≈ 30)
    RDom r(0, 30);

    // Expressões para o cálculo do meio e valor atual
    Expr meio = esquerda + (direita - esquerda) / 2;
    Expr valor_meio = arr(meio);

    // Atualização dos limites
    esquerda = select(valor_meio < valor, meio + 1, esquerda);
    direita = select(valor_meio > valor, meio - 1, direita);

    // Verificação de encontrado
    resultado = select(valor_meio == valor, meio, resultado);

    // Condição de parada
    Expr condicao_parada = (esquerda > direita) || (resultado != -1);

    // Ignora atualizações após encontrar o resultado
    meio = select(condicao_parada, meio, esquerda + (direita - esquerda) / 2);
    valor_meio = select(condicao_parada, valor_meio, arr(meio));

    return resultado;
}

int main() {
    double tempo_total = 0;

    for (int t = 0; t < NUM_TAMANHOS; t++) {
        const int TAMANHO_ARRAY = tamanhos[t];

        // Prepara os dados
        vector<int> arr(TAMANHO_ARRAY);
        vector<int> buscas(NUM_BUSCAS);

        // Preenche o array com valores pares ordenados
        for (int i = 0; i < TAMANHO_ARRAY; i++) {
            arr[i] = i * 2;
        }

        // Gera valores aleatórios para buscar
        mt19937 rng(random_device{}());
        uniform_int_distribution<int> dist(0, 2 * TAMANHO_ARRAY - 1);
        for (int i = 0; i < NUM_BUSCAS; i++) {
            buscas[i] = dist(rng);
        }

        // Cria buffers Halide
        Buffer<int> arr_buf(arr.data(), TAMANHO_ARRAY);
        Buffer<int> buscas_buf(buscas.data(), NUM_BUSCAS);
        Buffer<int> resultados_buf(NUM_BUSCAS);

        // Define o pipeline Halide
        Var x;
        Func arr_func("arr"), busca_func("busca"), resultado_func("resultado");

        arr_func(x) = arr_buf(x);

        // Converte o tamanho para Expr
        Expr tamanho_expr = cast<int>(TAMANHO_ARRAY);

        // Implementação vetorizada das buscas
        resultado_func(x) = busca_binaria_halide(arr_func, tamanho_expr, buscas_buf(x));

        // Otimizações de agendamento
        arr_func.compute_root();
        resultado_func.compute_root().parallel(x, 16).vectorize(x, 8);

        // Executa e mede o tempo
        auto start = high_resolution_clock::now();
        resultado_func.realize(resultados_buf);
        auto stop = high_resolution_clock::now();

        double tempo_execucao = duration_cast<duration<double>>(stop - start).count();
        tempo_total += tempo_execucao;

        cout << "Tamanho: " << TAMANHO_ARRAY
            << " - Tempo: " << tempo_execucao
            << " segundos" << endl;
    }

    cout << "Média do tempo de execução: " << tempo_total / NUM_TAMANHOS
        << " segundos" << endl;

    return 0;
}