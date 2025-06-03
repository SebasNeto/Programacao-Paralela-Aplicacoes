#include <Halide.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>

using namespace Halide;

// Função de partição original
int particionar(Buffer<int>& vetor, int baixo, int alto) {
    int pivo = vetor(alto);
    int i = baixo - 1;

    for (int j = baixo; j < alto; j++) {
        if (vetor(j) < pivo) {
            i++;
            std::swap(vetor(i), vetor(j));
        }
    }

    std::swap(vetor(i + 1), vetor(alto));
    return i + 1;
}

// Quicksort original adaptado para trabalhar com Buffer Halide
void quicksort(Buffer<int>& vetor, int baixo, int alto) {
    if (baixo < alto) {
        int pi = particionar(vetor, baixo, alto);
        quicksort(vetor, baixo, pi - 1);
        quicksort(vetor, pi + 1, alto);
    }
}

// Versão "paralelizável" do particionamento usando Halide (experimental)
void particionar_halide(Buffer<int>& vetor, int baixo, int alto, int* pos_pivo) {
    Var x;
    Func particao;

    // Esta é uma tentativa de expressar o particionamento em Halide
    // Note que isso não é verdadeiramente paralelizável devido às dependências de dados
    RDom r(baixo, alto - baixo);
    Expr pivo = vetor(alto);
    Expr cond = vetor(r) < pivo;

    // Contagem de elementos menores que o pivô
    Func contador;
    contador(x) = 0;
    contador(0) += select(cond, 1, 0);

    // Aplicar a partição - isso ainda precisaria ser implementado
    // Esta parte é apenas conceitual
    Buffer<int> resultado = contador.realize({ 1 });
    *pos_pivo = baixo + resultado(0);
}

int main() {
    // Tamanhos reduzidos para demonstração
    int tamanhos[] = { 10000000, 20000000, 30000000, 40000000, 50000000, 60000000, 70000000, 80000000, 90000000, 100000000 };
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);
    double tempo_total = 0.0;

    srand(time(NULL));

    for (int i = 0; i < num_tamanhos; i++) {
        int tamanho = tamanhos[i];
        Buffer<int> vetor(tamanho);

        // Preenchimento do vetor usando Halide (paralelizado)
        Var x;
        Func preencher;
        preencher(x) = cast<int>(random_float() * 100000.0f);
        preencher.realize(vetor);

        clock_t inicio = clock();

        // Chamada para o quicksort original adaptado
        quicksort(vetor, 0, tamanho - 1);

        clock_t fim = clock();

        double tempo_execucao = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
        tempo_total += tempo_execucao;

        printf("Tamanho: %d, Tempo: %.4f segundos\n", tamanho, tempo_execucao);

        // Verificação opcional da ordenação
        bool ordenado = true;
        for (int j = 1; j < tamanho && ordenado; j++) {
            if (vetor(j - 1) > vetor(j)) {
                ordenado = false;
            }
        }
        printf("Vetor %sordenado corretamente\n", ordenado ? "" : "NÃO ");
    }

    printf("\nTempo médio: %.4f segundos\n", tempo_total / num_tamanhos);
    return 0;
}