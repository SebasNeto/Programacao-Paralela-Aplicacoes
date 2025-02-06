#include <stdio.h>

void mergeSort(float v[], float vaux[], unsigned inicio, unsigned fim) {
    unsigned meio, i, j, k;
    if (inicio < fim) {
        meio = (inicio + fim) / 2;
        
        mergeSort(v, vaux, inicio, meio);
        mergeSort(v, vaux, meio + 1, fim);

        i = inicio;
        j = meio + 1;
        k = inicio;

        while (i <= meio && j <= fim) {
            if (v[i] < v[j]) {
                vaux[k] = v[i];
                i++;
            } else {
                vaux[k] = v[j];
                j++;
            }
            k++;
        }

        while (i <= meio) {
            vaux[k] = v[i];
            i++;
            k++;
        }

        while (j <= fim) {
            vaux[k] = v[j];
            j++;
            k++;
        }

        for (k = inicio; k <= fim; k++) {
            v[k] = vaux[k];
        }
    }
}

int main() {
    float v[] = {5.4, 3.1, 9.8, 2.7, 6.0, 8.2, 1.5};
    unsigned n = sizeof(v) / sizeof(v[0]);
    float vaux[n];

    printf("Vetor original: ");
    for (unsigned i = 0; i < n; i++) {
        printf("%.2f ", v[i]);
    }
    printf("\n");

    mergeSort(v, vaux, 0, n - 1);

    printf("Vetor ordenado: ");
    for (unsigned i = 0; i < n; i++) {
        printf("%.2f ", v[i]);
    }
    printf("\n");

    return 0;
}
