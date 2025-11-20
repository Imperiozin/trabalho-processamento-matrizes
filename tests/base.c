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
            matriz1[linha][coluna] = 2;//rand() % 2;
            matriz2[linha][coluna] = 2;//rand() % 2;
        }
    }
}

// Função para multiplicar matrizes
// make the later definition become the actual implementation
void multiplicar_matrizes_impl() {
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

// wrapper that calls the actual implementation and returns checksum
unsigned long long multiplicar_matrizes(void) {
    clock_t ini, end;
    ini = clock();
    multiplicar_matrizes_impl();
    end = clock();

    //printf("%.2f s\n", (float) (end - ini) / CLOCKS_PER_SEC);

    unsigned long long checksum = 0ULL;
    for (int i = 0; i < NUMERO; ++i) {
        for (int j = 0; j < NUMERO; ++j) {
            checksum += (unsigned long long) resultado[i][j];
        }
    }
    return checksum;
}


int main() {
    gerar_matrizes();
    
    long long checksum = multiplicar_matrizes();

    fprintf(stdout, "%lld", checksum);

    return 0;
}