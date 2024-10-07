//MergeSort
//Video 42
//Implementação da inercalação

#include <string.h>
#include <stdio.h>

int main() {
    printf("Hello, World!\n");
    return 0;
}

void mergeSort(float v[],int vaux[], unsigned inicio, unsigned fim){
    unsigned meio,i,j,k;
    if(inicio < fim){ //se ainda não chegou no fim
        meio = (inicio + fim)/2; //calcular o meio para separar e fazer o MergeSort dos dois lados separados
        mergeSort(v,vaux, inicio, meio); //primeira metade
        mergeSort((v,vaux,meio+1, fim);//segunda metade
        //fazer intercalação agora
        //A linha de código acima foi para aplicar o MergeSort - separar os vetores para ordenar e depois intercalar
        i = inicio;
        j = meio + 1;
        k = inicio; //indices que vÃ£o percorrer os vetores
        while ((i <= meio) && (j <= fim)){ //enquanto os indices nÃ£o chegaram no final do vetor
            if(v[i] < v[j]){ //verifica quem Ã© menor na posiÃ§Ã£o i e j
                vaux[k] = v[i]; // como a posiÃ§Ã£o i Ã© menor o novo vetor recebe o valor do indice na atual posÃ§Ã£o
                i++; //os indices i e j devem andar atÃ© o final do vetor
            }
            else{ //caso o i nÃ£o seja menor, entÃ£o o j vai ser
                vaux[k] = v[j]; //o vetor ordenado recebe o valor do indice j
                j++;}
            while (i <= meio){ //caso sobre elementos quando os indices chegarem no final do vetor
                vaux[k] = v[i]; //copiar eles para o vetor ordenado
                i++;k++;
            }
            while((j <= fim)){
                vaux[k] = v[j];
                j++; K++;
            }
            for (k <= inicio;k <= fim; K++){ //Caso sobre algum elemento copiar pro vetor ordenado
                v[k] = vaux[k];
            }

        }
    }
}