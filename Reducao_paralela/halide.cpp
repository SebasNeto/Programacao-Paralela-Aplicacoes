#include <Halide.h>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

using namespace Halide;
using namespace std;
using namespace std::chrono;

const int NUM_ITER = 10;
const int tamanhos[] = {
    10000000, 20000000, 30000000, 40000000, 50000000,
    60000000, 70000000, 80000000, 90000000, 100000000
};
const int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);

int main() {
    // Inicializa gerador de números aleatórios
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 9);

    double tempo_total = 0.0;
    volatile long long soma_total_dummy = 0;

    cout << "Teste de Redução Paralela com Halide" << endl;
    cout << "Número de iterações por tamanho: " << NUM_ITER << endl << endl;

    for (int t = 0; t < num_tamanhos; t++) {
        const int n = tamanhos[t];
        vector<int> vetor(n);

        // Preencher vetor com números aleatórios
        for (int i = 0; i < n; i++) {
            vetor[i] = dis(gen);
        }

        Buffer<int> input(vetor.data(), n);
        double tempo_medio = 0.0;

        for (int iter = 0; iter < NUM_ITER; iter++) {
            auto start = high_resolution_clock::now();

            // Definir o pipeline Halide
            Var x;
            RDom r(0, (int)n);  // Conversão explícita para int

            // Operação de redução (soma)
            Func reducao;
            reducao() = sum(cast<int64_t>(input(r)));  // Usar int64_t para evitar overflow

            // Agendamento otimizado
            reducao.compute_root();
            input.set_host_dirty();  // Indica que os dados foram modificados no host

            // Executar o pipeline
            Buffer<int64_t> resultado = reducao.realize();

            auto end = high_resolution_clock::now();
            double tempo_exec = duration_cast<duration<double>>(end - start).count();
            tempo_medio += tempo_exec;

            soma_total_dummy += resultado();
        }

        tempo_medio /= NUM_ITER;
        tempo_total += tempo_medio;
        cout << "Tamanho do vetor: " << n << " -> Tempo médio: " << tempo_medio << " segundos" << endl;
    }

    double tempo_medio_geral = tempo_total / num_tamanhos;
    cout << "\nTempo médio geral: " << tempo_medio_geral << " segundos" << endl;
    cout << "Soma total (dummy): " << soma_total_dummy << endl;

    return 0;
}