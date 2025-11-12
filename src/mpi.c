#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <mpi.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#ifndef NUMERO
#define NUMERO 4000
#endif

#ifndef TILE
#define TILE 128
#endif

static inline int min_i(int a, int b) { return a < b ? a : b; }

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int N = NUMERO;
    if (N % size != 0) {
        if (rank == 0) fprintf(stderr, "N (%d) deve ser múltiplo de size (%d).\n", N, size);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    const int rows_local = N / size;
    const size_t bytesA_local = (size_t)rows_local * N * sizeof(int32_t);
    const size_t bytesB       = (size_t)N * N * sizeof(int32_t);
    const size_t bytesC_local = (size_t)rows_local * N * sizeof(int32_t);

    int32_t *A_root = NULL;
    int32_t *B      = (int32_t*)malloc(bytesB);
    int32_t *BT     = (int32_t*)malloc(bytesB);               // transposta local de B
    int32_t *A_loc  = (int32_t*)malloc(bytesA_local);
    int32_t *C_loc  = (int32_t*)malloc(bytesC_local);

    if (!B || !BT || !A_loc || !C_loc) {
        fprintf(stderr, "[%d] Falha malloc.\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Rank 0 gera dados
    if (rank == 0) {
        A_root = (int32_t*)malloc((size_t)N * N * sizeof(int32_t));
        if (!A_root) { fprintf(stderr, "Falha malloc A_root.\n"); MPI_Abort(MPI_COMM_WORLD, 1); }
        srand(12345u);
        for (int i = 0; i < N*N; ++i) {
            A_root[i] = rand() % 2;
            B[i]      = rand() % 2;
        }
    }

    // Scatter das linhas de A
    MPI_Scatter(A_root, rows_local * N, MPI_INT, A_loc, rows_local * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Broadcast de B completa
    MPI_Bcast(B, N*N, MPI_INT, 0, MPI_COMM_WORLD);

    // Transposição local de B -> BT
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            BT[(size_t)j*N + i] = B[(size_t)i*N + j];

    // Zera C_local
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < rows_local * N; ++i) C_loc[i] = 0;

    // Sincroniza antes de cronometrar
    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    // Multiplicação bloqueada: somente nas linhas deste rank
    #pragma omp parallel for collapse(2) schedule(static)
    for (int ii = 0; ii < rows_local; ii += TILE) {
        for (int jj = 0; jj < N; jj += TILE) {
            for (int kk = 0; kk < N; kk += TILE) {
                const int i_max = min_i(ii + TILE, rows_local);
                const int j_max = min_i(jj + TILE, N);
                const int k_max = min_i(kk + TILE, N);

                for (int i = ii; i < i_max; ++i) {
                    const int32_t *Ai = &A_loc[(size_t)i*N + kk];
                    int32_t       *Ci = &C_loc[(size_t)i*N + jj];

                    for (int j = jj; j < j_max; ++j) {
                        const int32_t *BTj = &BT[(size_t)j*N + kk];
                        int sum = 0;

                        #pragma omp simd reduction(+:sum)
                        for (int k = kk; k < k_max; ++k) {
                            sum += Ai[k - kk] * BTj[k - kk];
                        }
                        Ci[j - jj] += sum;
                    }
                }
            }
        }
    }

    double t1 = MPI_Wtime();
    double mytime = t1 - t0, maxtime = 0.0;
    MPI_Reduce(&mytime, &maxtime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    // Reúne C no rank 0
    if (rank == 0) {
        int32_t *C_root = (int32_t*)malloc((size_t)N * N * sizeof(int32_t));
        if (!C_root) { fprintf(stderr, "Falha malloc C_root.\n"); MPI_Abort(MPI_COMM_WORLD, 1); }
        MPI_Gather(C_loc, rows_local * N, MPI_INT, C_root, rows_local * N, MPI_INT, 0, MPI_COMM_WORLD);

        // checksum para evitar DCE
        long long checksum = 0;
        for (int i = 0; i < N*N; ++i) checksum += C_root[i];
        printf("tempo(parede)=%.3f s  checksum=%lld\n", maxtime, checksum);
        free(C_root);
        free(A_root);
    } else {
        MPI_Gather(C_loc, rows_local * N, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
    }

    free(B);
    free(BT);
    free(A_loc);
    free(C_loc);
    MPI_Finalize();
    return 0;
}