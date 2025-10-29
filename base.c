#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUMERO 2000

int matriz1[NUMERO][NUMERO];
int matriz2[NUMERO][NUMERO];
int resultado[NUMERO][NUMERO];

// Função para gerar matrizes com valores aleatórios
void gerar_matrizes() {
    for (int linha = 0; linha < NUMERO; linha++) {
        for (int coluna = 0; coluna < NUMERO; coluna++) {
            matriz1[linha][coluna] = rand() % 2;
            matriz2[linha][coluna] = rand() % 2;
        }
    }
}

// Função para multiplicar matrizes
void multiplicar_matrizes() {
    for (int linha = 0; linha < NUMERO; linha++) {
        for (int coluna = 0; coluna < NUMERO; coluna++) {
            int soma = 0;
            for (int k = 0; k < NUMERO; k++) {
                soma += matriz1[linha][k] * matriz2[k][coluna];
            }
            resultado[linha][coluna] = soma;
        }
    }
}

int main() {
    clock_t ini, end;

    gerar_matrizes();

    ini = clock();

    multiplicar_matrizes();

    end = clock();

    printf("%.2f s\n", (float) (end - ini) / CLOCKS_PER_SEC);

    return 0;
}