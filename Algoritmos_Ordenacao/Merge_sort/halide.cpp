#include <Halide.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include <numeric>

using namespace Halide;
using namespace std;
using namespace std::chrono;

const int PARALLEL_THRESHOLD = 1 << 20;  // Limiar para paralelismo (1M elementos)

bool is_sorted(const vector<int>& arr) {
    for (size_t i = 1; i < arr.size(); i++)
        if (arr[i - 1] > arr[i]) return false;
    return true;
}

void serial_merge(Buffer<int>& buf, int left, int mid, int right) {
    vector<int> temp(right - left + 1);
    int i = left, j = mid + 1, k = 0;

    while (i <= mid && j <= right) {
        temp[k++] = buf(i) <= buf(j) ? buf(i++) : buf(j++);
    }
    while (i <= mid) temp[k++] = buf(i++);
    while (j <= right) temp[k++] = buf(j++);

    for (int idx = 0; idx < k; idx++) {
        buf(left + idx) = temp[idx];
    }
}

void merge_sort_halide(Buffer<int>& buf, int left, int right) {
    if (left >= right) return;

    // Usar insertion sort para pequenos intervalos
    if (right - left < 32) {
        for (int i = left + 1; i <= right; i++) {
            int key = buf(i);
            int j = i - 1;
            while (j >= left && buf(j) > key) {
                buf(j + 1) = buf(j);
                j--;
            }
            buf(j + 1) = key;
        }
        return;
    }

    int mid = left + (right - left) / 2;

    // Divisão recursiva
    if (right - left > PARALLEL_THRESHOLD) {
        // Versão paralela para grandes intervalos
        Var x;
        Func left_sort, right_sort;

        // Ordena a metade esquerda
        left_sort(x) = buf(x);
        left_sort.compute_root().parallel(x);
        merge_sort_halide(buf, left, mid);

        // Ordena a metade direita
        right_sort(x) = buf(x);
        right_sort.compute_root().parallel(x);
        merge_sort_halide(buf, mid + 1, right);
    }
    else {
        // Versão serial para pequenos intervalos
        merge_sort_halide(buf, left, mid);
        merge_sort_halide(buf, mid + 1, right);
    }

    // Mesclagem - usando versão serial para maior estabilidade
    serial_merge(buf, left, mid, right);
}

int main() {
    vector<int> sizes = { 10000000, 20000000, 30000000, 40000000, 50000000, 60000000, 70000000, 80000000, 90000000, 100000000 };
    vector<double> times;

    cout << "Iniciando testes do Merge Sort em Halide..." << endl;

    for (int size : sizes) {
        cout << "Preparando vetor de tamanho " << size << "..." << flush;

        vector<int> data(size);
        mt19937 rng(random_device{}());
        uniform_int_distribution<int> dist(1, size);
        generate(data.begin(), data.end(), [&]() { return dist(rng); });

        Buffer<int> buf(size);
        for (int i = 0; i < size; i++) {
            buf(i) = data[i];
        }

        cout << " Ordenando..." << flush;
        auto start = high_resolution_clock::now();
        merge_sort_halide(buf, 0, size - 1);
        auto stop = high_resolution_clock::now();

        for (int i = 0; i < size; i++) {
            data[i] = buf(i);
        }

        auto duration = duration_cast<milliseconds>(stop - start);
        double seconds = duration.count() / 1000.0;
        times.push_back(seconds);

        if (!is_sorted(data)) {
            cerr << "\nErro: array não ordenado para tamanho " << size << endl;
            return 1;
        }

        cout << " Concluído: " << seconds << " segundos" << endl;
    }

    double average = accumulate(times.begin(), times.end(), 0.0) / times.size();
    cout << "\nTempo médio de execução: " << average << " segundos" << endl;

    return 0;
}