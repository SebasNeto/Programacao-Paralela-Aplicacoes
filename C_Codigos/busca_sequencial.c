#include <stdio.h>
#include <string.h>

// parametros necessário: vetor[], tamanho do vetor, chave de bsuca

int buscaSequencial(float vet [], unsigned tam, int chave){ //chave de busca
    unsigned x;
    for(x=0;x<tam;x++){
        if(vet[x] == chave){
            return 1;
        }
    }
    return 0;
}

typedef struct tipoAluno{
    char nome[50];
    int matricula;
    char curso[30];
}tipoAluno;


int buscaAluno(tipoAluno vet[], unsigned tam, char chave[]){ //nesse caso a chave é caracteres
    unsigned x;
    for(x=0; x< tam; x++){ //andando com o vetor - posição do vetor
        if(strcmp(vet[x].nome, chave)==0){ //compara o vetor na posição nome da struct com a chave solicitada
            return x;
        }
    }
    return -1;
}

int main() {
    float vet1[5] = {3.8,6.7,12,25,9.1};
    float valor;

    tipoAluno aluno [2];
    char chave[40];
    int pos; //armazenar a posição da busca

    strcpy(aluno[0].nome,"SEBASTIAO BICHARRA NETO" );
    strcpy(aluno[0].curso, "CIENCIA DA COMPUTACAO");
    strcpy(aluno[1].nome,"XXXXX" );
    strcpy(aluno[1].curso, "YYYYY");

/*
    printf("entre cm uma chave de busca: \n");
    scanf("%f%*c",&valor);
    if(buscaSequencial(vet1,5,valor)){
        printf("chave encontrada \n");
    }
    else{
        printf("chave nao encontrada \n");
    } */

    int x;

    for(x=0; x<2; x++ ){
        aluno[x].matricula = x;
    }
    printf("digite uma chave: \n");
    scanf("%s", chave);

    pos = (buscaAluno(aluno, 2, chave) != -1); //achou
        if (pos != -1){
            printf("Curso: %s \n", aluno[pos].curso);
            printf("matricula: %s \n", aluno[pos].matricula);
        }
        else{
            printf("aluno nao foi encontrado \n");
        }
}