#include "Halide.h"
#include "halide_benchmark.h"
#include <cstdio>
#include <vector>

using namespace Halide;
using namespace Halide::Tools;

int main(int argc, char** argv) {

    std::vector<int> tamanhos = { 256, 512, 1024, 2048 };

    Target target = get_jit_target_from_environment();
    const int vec = target.natural_vector_size<float>();

    double soma_tempos = 0.0;
    int total_testes = tamanhos.size();

    for (int tamanho : tamanhos) {
        printf("Benchmark com matrizes de tamanho: %dx%d\n", tamanho, tamanho);

        Buffer<float> A(tamanho, tamanho);
        Buffer<float> B(tamanho, tamanho);
        Buffer<float> C(tamanho, tamanho);

        for (int y = 0; y < tamanho; y++) {
            for (int x = 0; x < tamanho; x++) {
                A(x, y) = (rand() % 100) / 100.0f;
                B(x, y) = (rand() % 100) / 100.0f;
            }
        }

        Var x("x"), y("y");
        Func mat_mul("mat_mul");
        RDom k(0, tamanho);

        mat_mul(x, y) = sum(A(k, y) * B(x, k));

        Var xo, yo, xi, yi;
        mat_mul.tile(x, y, xo, yo, xi, yi, 24, 24) 
            .fuse(xo, yo, xo)                
            .parallel(xo);                


        mat_mul.update()
            .reorder(x, y, k)     
            .vectorize(x, vec)    
            .unroll(k, 4);      

        // Medir tempo de execução
        double tempo = benchmark([&]() {
            mat_mul.realize(C);
            });

        printf("Tempo de execução: %.6f segundos\n", tempo);
        printf("----------\n");

        soma_tempos += tempo;
    }

    double media = soma_tempos / total_testes;
    printf("Tempo médio de execução: %.6f segundos\n", media);

    return 0;
}






