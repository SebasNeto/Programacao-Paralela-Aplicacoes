#include "Halide.h"
#include "halide_benchmark.h"
#include <cstdio>
#include <vector>

using namespace Halide;
using namespace Halide::Tools;

int main(int argc, char** argv) {
    std::vector<int> tamanhos = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};

    Target target = get_jit_target_from_environment();
    const int vec = target.natural_vector_size<float>();

    double soma_tempos = 0.0;
    int total_testes = tamanhos.size();

    for (int tamanho : tamanhos) {
        printf("Benchmark com vetor de tamanho: %d\n", tamanho);

        Buffer<float> A(tamanho), B(tamanho), C(tamanho);

        for (int i = 0; i < tamanho; i++) {
            A(i) = (rand() % 100) / 100.0f;
            B(i) = (rand() % 100) / 100.0f;
        }

        Var x("x");
        Func vetor_mul("vetor_mul");

        vetor_mul(x) = A(x) * B(x);
        vetor_mul.vectorize(x, vec).parallel(x);

        // Medir tempo de execução
        double tempo = benchmark([&]() {
            vetor_mul.realize(C);
            });

        printf("Tempo de execução: %.6f segundos\n", tempo);
        printf("----------\n");

        soma_tempos += tempo;
    }

    double media = soma_tempos / total_testes;
    printf("Tempo médio de execução: %.6f segundos\n", media);

    return 0;
}



Var x("x");
Func prod("prod");                      // C(x) = A(x)*B(x)
prod(x) = A(x) * B(x);                  // expressão pura

prod.vectorize(x, vec)                 // SIMD: 'vec' = largura natural
    .parallel(x);                      // divide o domínio x entre threads
prod.realize(C);                       // gera resultado em buffer C




