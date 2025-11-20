#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>

#ifndef NUMERO
    #define NUMERO 4000
#endif

#ifndef TILE
    #define TILE 128
#endif

static inline void *xaligned_alloc(size_t align, size_t bytes)
{
    #if defined(_WIN32)
        return _aligned_malloc(bytes, align);
    #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
        // aligned_alloc exige múltiplo de align
        size_t sz = (bytes + (align - 1)) & ~(align - 1);
        return aligned_alloc(align, sz);
    #else
        void *p = NULL;
        if (posix_memalign(&p, align, bytes) != 0)
            return NULL;
        return p;
    #endif

    return malloc(bytes);
}

static inline void xaligned_free(void *p)
{
#if defined(_WIN32)
    _aligned_free(p);
#else
    free(p);
#endif
}

static inline int min_i(int a, int b) { return a < b ? a : b; }

int _seed = 42;

void generate_matrix(int32_t *A, int32_t *B, int N)
{
    // Geração dos dados
    srand(_seed);

    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            A[(size_t)i * N + j] = rand() % 2;
            B[(size_t)i * N + j] = rand() % 2;
        }
    }
}

void calculate_matrix(int32_t *A, int32_t *B, int32_t *BT, int32_t *C, int N)
{
    // const double t0 = omp_get_wtime();
    // Transpor B -> BT para acesso contíguo no loop interno
    #pragma omp parallel for collapse(2) schedule(static)
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                BT[(size_t)j * N + i] = B[(size_t)i * N + j];

    // Zerar C
    #pragma omp parallel for schedule(static)
        for (int i = 0; i < N * N; i++)
            C[i] = 0;

    // Multiplicação bloqueada: C += A * B, usando BT
    // Paralelismo nos blocos externos, vetoriza o loop de k
    #pragma omp parallel for collapse(2) schedule(static)
        for (int ii = 0; ii < N; ii += TILE)
        {
            for (int jj = 0; jj < N; jj += TILE)
            {
                for (int kk = 0; kk < N; kk += TILE)
                {
                    const int i_max = min_i(ii + TILE, N);
                    const int j_max = min_i(jj + TILE, N);
                    const int k_max = min_i(kk + TILE, N);

                    for (int i = ii; i < i_max; i++)
                    {
                        const int32_t *Ai = &A[(size_t)i * N + kk];
                        int32_t *Ci = &C[(size_t)i * N + jj];

                        for (int j = jj; j < j_max; j++)
                        {
                            const int32_t *BTj = &BT[(size_t)j * N + kk];
                            int sum = 0;

    #pragma omp simd reduction(+ : sum)
                        for (int k = kk; k < k_max; k++)
                        {
                            // Ai[k-kk] == A[i, k], BTj[k-kk] == B[k, j]
                            sum += Ai[k - kk] * BTj[k - kk];
                        }
                        Ci[j - jj] += sum;
                    }
                }
            }
        }
    }

    // const double t1 = omp_get_wtime();
    // printf("%.2f s\n", t1 - t0);
}

void calculate_checksum(int32_t *C, int N)
{
    long long checksum = 0;
    #pragma omp parallel for reduction(+ : checksum) schedule(static)
    for (int i = 0; i < N * N; i++)
        checksum += C[i];

    fprintf(stdout, "%lld", checksum);
}

int main(int argc, char **argv)
{
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

    const size_t bytes = (size_t)N * (size_t)N * sizeof(int32_t);

    // Alocação alinhada para melhor SIMD e pré-busca
    int32_t *A = (int32_t *)xaligned_alloc(64, bytes);
    int32_t *B = (int32_t *)xaligned_alloc(64, bytes);
    int32_t *BT = (int32_t *)xaligned_alloc(64, bytes); // B transposta
    int32_t *C = (int32_t *)xaligned_alloc(64, bytes);

    if (!A || !B || !BT || !C)
    {
        fprintf(stderr, "Falha ao alocar memoria\n");
        return 1;
    }

    generate_matrix(A, B, N);

    calculate_matrix(A, B, BT, C, N);

    // Evita que o compilador descarte C
    calculate_checksum(C, N);

    xaligned_free(A);
    xaligned_free(B);
    xaligned_free(BT);
    xaligned_free(C);
    return 0;
}
