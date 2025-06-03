#include <Halide.h>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

using namespace Halide;
using namespace std;
using namespace std::chrono;

const int NUM_ITERACOES = 1;
const int tamanhos_amostras[] = {
    10000000, 20000000, 30000000, 40000000, 50000000,
    60000000, 70000000, 80000000, 90000000, 100000000
};
const int NUM_TAMANHOS = sizeof(tamanhos_amostras) / sizeof(tamanhos_amostras[0]);

int main() {
    // Inicializa gerador de números aleatórios
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(-1.0, 1.0);

    for (int t = 0; t < NUM_TAMANHOS; t++) {
        const int NUM_AMOSTRAS = tamanhos_amostras[t];
        double tempo_total = 0.0;

        cout << "\nTamanho da amostra: " << NUM_AMOSTRAS << endl;

        for (int iteracao = 0; iteracao < NUM_ITERACOES; iteracao++) {
            auto start = high_resolution_clock::now();

            // Criar buffers para coordenadas x e y
            Buffer<double> x_coords(NUM_AMOSTRAS);
            Buffer<double> y_coords(NUM_AMOSTRAS);

            // Preencher buffers com números aleatórios
            for (int i = 0; i < NUM_AMOSTRAS; i++) {
                x_coords(i) = dis(gen);
                y_coords(i) = dis(gen);
            }

            // Definir o pipeline Halide
            Var i;
            Func dentro_circulo;

            // Expressão para verificar se o ponto está dentro do círculo
            Expr x = x_coords(i);
            Expr y = y_coords(i);
            dentro_circulo(i) = cast<int>(x * x + y * y <= 1.0f);

            // Contar pontos dentro do círculo
            RDom r(0, NUM_AMOSTRAS);
            Func contador;
            contador() = sum(dentro_circulo(r));

            // Otimizações de agendamento
            dentro_circulo.compute_root().parallel(i);
            contador.compute_root();

            // Executar o pipeline
            Buffer<int> resultado = contador.realize();

            auto end = high_resolution_clock::now();
            double tempo_decorrido = duration_cast<duration<double>>(end - start).count();
            tempo_total += tempo_decorrido;

            int contador_valor = resultado();
            double pi_estimado = 4.0 * contador_valor / NUM_AMOSTRAS;

            cout << "Iteração " << iteracao + 1 << ": pi = " << pi_estimado
                << ", tempo = " << tempo_decorrido << " segundos" << endl;
        }

        cout << "Tempo médio para amostra " << NUM_AMOSTRAS << ": "
            << tempo_total / NUM_ITERACOES << " segundos" << endl;
    }

    return 0;
}