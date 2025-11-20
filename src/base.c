#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUMERO 2000

int32_t* matriz1;
int32_t* matriz2;
int32_t* resultado;

int _seed = 42;

// Função para gerar matrizes com valores aleatórios
void gerar_matrizes(int length) {
    srand(_seed);

    for (int linha = 0; linha < length; linha++) {
        for (int coluna = 0; coluna < length; coluna++) {
            matriz1[linha * length + coluna] = rand() % 2;
            matriz2[linha * length + coluna] = rand() % 2;
        }
    }
}

// Função para multiplicar matrizes
void multiplicar_matrizes(int length) {
    //clock_t ini, end;
    //ini = clock();
    
    for (int linha = 0; linha < length; linha++) {
        for (int coluna = 0; coluna < length; coluna++) {
            int32_t soma = 0;
            for (int k = 0; k < length; k++) {
                soma += matriz1[linha * length + k] * matriz2[k * length + coluna];
            }
            resultado[linha * length + coluna] = soma;
        }
    }
    //end = clock();
    //printf("%.2f s\n", (float) (end - ini) / CLOCKS_PER_SEC);
}

long long calculate_checksum(int length)
{
    long long checksum = 0;
    for (int i = 0; i < length; ++i) {
        for (int j = 0; j < length; ++j) {
            checksum += (long long) resultado[i * length + j];
        }
    }

    return checksum;
}

int main(int argc, char **argv) {
    char* numero_str = NULL;

    if(argc > 0)
    {
        numero_str = argv[1];

        if(argc > 1)
        {
            _seed = atoi(argv[2]);
        }
    }

    const int N = numero_str ? atoi(numero_str) : NUMERO;

    matriz1 = malloc(N * N * sizeof(int32_t));
    matriz2 = malloc(N * N * sizeof(int32_t));
    resultado = malloc(N * N * sizeof(int32_t));

    gerar_matrizes(N);

    multiplicar_matrizes(N);

    fprintf(stdout, "%lld", calculate_checksum(N));

    return 0;
}