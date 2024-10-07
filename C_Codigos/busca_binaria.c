#include <stdio.h>
#include <time.h>

int buscaBinaria(float vet[], unsigned tam, float chave, int *compara){
    int inicio, meio, fim;
    inicio = 0;
    fim = tam - 1;
    *compara = 0; 
    while(inicio <= fim){ 
        meio = (inicio + fim) / 2;
        (*compara)++;
        if(chave > vet[meio]){ 
            inicio = meio + 1;
        }
        else if(chave < vet[meio]){ 
            fim = meio - 1;
        }
        else{
            return meio; //índice da chave encontrada
        }
    }
    return -1; //chave não encontrada
}

int main() {
    float vetor[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    unsigned tamanho = sizeof(vetor) / sizeof(vetor[0]);
    float chave;
    int resultado;
    int compara;
    clock_t start_time, end_time;
    double time_taken;

    printf("Digite o valor a ser buscado: ");
    scanf("%f", &chave);

    start_time = clock(); // Inicia a contagem de tempo
    resultado = buscaBinaria(vetor, tamanho, chave, &compara);
    end_time = clock(); // Termina a contagem de tempo

    time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    if (resultado != -1) {
        printf("Valor %.2f encontrado na posição %d.\n", chave, resultado);
    } else {
        printf("Valor %.2f não encontrado no vetor.\n", chave);
    }

    printf("Número de comparações: %d\n", compara);
    printf("Tempo de execução: %.8f segundos\n", time_taken);

    return 0;
}