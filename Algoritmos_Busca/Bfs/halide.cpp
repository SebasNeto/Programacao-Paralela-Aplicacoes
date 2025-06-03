#include <Halide.h>
#include <iostream>
#include <random>
#include <vector>
#include <chrono>

using namespace Halide;
using namespace std;
using namespace std::chrono;

const int MAX_GRAU = 10;
const int MAX_ITERS = 30;

int main() {
    vector<long> tamanhos = { 1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 7000000, 8000000, 9000000 };
    double soma_tempos = 0.0;

    for (long n : tamanhos) {
        // ---------- Gerar grafo aleatório fixo ----------
        vector<vector<int>> vizinhos(n, vector<int>(MAX_GRAU, -1));
        default_random_engine gen;
        uniform_int_distribution<int> dist(0, n - 1);

        for (long i = 0; i < n; i++) {
            for (int j = 0; j < MAX_GRAU; j++) {
                int v = dist(gen);
                while (v == i) v = dist(gen); // evitar laços
                vizinhos[i][j] = v;
            }
        }

        // ---------- Buffers ----------
        Buffer<int> viz(n, MAX_GRAU);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < MAX_GRAU; j++)
                viz(i, j) = vizinhos[i][j];

        Buffer<int> distancia_atual(n);
        distancia_atual.fill(-1);
        distancia_atual(0) = 0; // origem

        // ---------- Variáveis Halide ----------
        Var i;
        RDom r(0, MAX_GRAU);
        Func atualizar("atualizar");

        // Garante que os vizinhos acessados estão dentro dos limites de 0 a n - 1
        Expr vizinho = clamp(viz(i, r), 0, (int)n - 1);
        Expr distancia_viz = select(
            distancia_atual(vizinho) >= 0,
            distancia_atual(vizinho) + 1,
            1 << 30
        );

        Expr menor_distancia = minimum(distancia_viz);

        atualizar(i) = select(
            distancia_atual(i) == -1,
            menor_distancia,
            distancia_atual(i)
        );

        atualizar.parallel(i);

        // ---------- Executar iterações ----------
        auto inicio = high_resolution_clock::now();

        for (int iter = 0; iter < MAX_ITERS; iter++) {
            Buffer<int> out(n);
            atualizar.realize(out);
            distancia_atual = out;

        }

        auto fim = high_resolution_clock::now();
        double tempo = duration<double>(fim - inicio).count();
        soma_tempos += tempo;

        cout << "Tamanho: " << n << " vértices - Tempo: " << tempo << " s\n";
    }

    double media = soma_tempos / tamanhos.size();
    cout << "Tempo médio (paralelo Halide): " << media << " s\n";

    return 0;
}
