#!/bin/bash

# Compilar o programa
gcc -o multiplicacao_matrizes script.c -O2

# Testar para tamanhos variados de matrizes
for N in $(seq 1000 1000 10000); do
    ./multiplicacao_matrizes $N
done
