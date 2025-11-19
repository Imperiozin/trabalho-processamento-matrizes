// mpicc -O3 -fopenmp -march=native guirafa.c -o guirafa
// mpirun -hostfile hosts -np 8 --bind-to core --map-by ppr:1:socket ./guirafa

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

// TILE padrão; pode sobrescrever com env TILE=96,128,...
#ifndef TILE
#define TILE 128
#endif

static inline int min_i(int a, int b) { return a < b ? a : b; }

static int pick_tile(void) {
    const char *s = getenv("TILE");
    if (s) {
        int t = atoi(s);
        if (t > 0) return t;
    }
    return TILE;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

#ifdef _OPENMP
    // Fixar threads por processo (opcional; pode ajustar externamente)
    // omp_set_dynamic(0);
#endif

    const int N = NUMERO;
    const int T = pick_tile();

    // Distribuição de linhas (não requer N % size == 0)
    int *counts = (int*)malloc(size * sizeof(int));
    int *displs = (int*)malloc(size * sizeof(int));
    if (!counts || !displs) {
        if (rank == 0) fprintf(stderr, "Falha malloc counts/displs\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    const int base = N / size;
    const int extra = N % size;
    for (int r = 0; r < size; ++r) {
        int rows = base + (r < extra ? 1 : 0);
        counts[r] = rows; // em linhas
    }
    displs[0] = 0;
    for (int r = 1; r < size; ++r) displs[r] = displs[r-1] + counts[r-1];

    const int rows_local = counts[rank];
    const size_t bytesA_local = (size_t)rows_local * N * sizeof(int32_t);
    const size_t bytesB       = (size_t)N * N * sizeof(int32_t);
    const size_t bytesC_local = (size_t)rows_local * N * sizeof(int32_t);

    int32_t *A_root = NULL;
    int32_t *B      = (int32_t*)malloc(bytesB);
    int32_t *BT     = (int32_t*)malloc(bytesB);               // transposta local de B
    int32_t *A_loc  = (int32_t*)malloc(bytesA_local);
    int32_t *C_loc  = (int32_t*)malloc(bytesC_local);

    if (!B || !BT || !A_loc || !C_loc) {
        fprintf(stderr, "[%d] Falha malloc (B/BT/A_loc/C_loc)\n", rank);
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

    // Scatterv das linhas de A (em blocos contíguos de linhas)
    // counts/displs estão em linhas → converter para elementos
    int *sendcounts_elems = (int*)malloc(size * sizeof(int));
    int *displs_elems     = (int*)malloc(size * sizeof(int));
    if (!sendcounts_elems || !displs_elems) {
        if (rank == 0) fprintf(stderr, "Falha malloc sendcounts/displs elems\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    for (int r = 0; r < size; ++r) {
        sendcounts_elems[r] = counts[r] * N;
        displs_elems[r]     = displs[r] * N;
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Scatterv(A_root, sendcounts_elems, displs_elems, MPI_INT32_T,
                 A_loc, rows_local * N, MPI_INT32_T, 0, MPI_COMM_WORLD);

    // Broadcast de B completa
    MPI_Bcast(B, N*N, MPI_INT32_T, 0, MPI_COMM_WORLD);

    // Transposição local de B -> BT (row-major)
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
BT[(size_t)j*N + i] = B[(size_t)i*N + j];

// Zera C_local
#pragma omp parallel for schedule(static)
for (int i = 0; i < rows_local * N; ++i) C_loc[i] = 0;

// Sincroniza antes de cronometrar
    double t0 = MPI_Wtime();

    // Multiplicação bloqueada: somente nas linhas deste rank
    #pragma omp parallel for collapse(2) schedule(static)
    for (int ii = 0; ii < rows_local; ii += T) {
        for (int jj = 0; jj < N; jj += T) {
            for (int kk = 0; kk < N; kk += T) {
                const int i_max = min_i(ii + T, rows_local);
                const int j_max = min_i(jj + T, N);
                const int k_max = min_i(kk + T, N);

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

    // Reúne C no rank 0 (Gatherv)
    int32_t *C_root = NULL;
    if (rank == 0) {
        C_root = (int32_t*)malloc((size_t)N * N * sizeof(int32_t));
        if (!C_root) { fprintf(stderr, "Falha malloc C_root.\n"); MPI_Abort(MPI_COMM_WORLD, 1); }
    }

    MPI_Gatherv(C_loc, rows_local * N, MPI_INT32_T,
                C_root, sendcounts_elems, displs_elems, MPI_INT32_T,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        // checksum para evitar DCE
        long long checksum = 0;
        for (int i = 0; i < N*N; ++i) checksum += C_root[i];
        printf("N=%d TILE=%d  tempo(parede)=%.3f s  checksum=%lld\n", N, T, maxtime, checksum);
    }

    // Libera
    if (rank == 0) { free(A_root); free(C_root); }
    free(B); free(BT); free(A_loc); free(C_loc);
    free(counts); free(displs); free(sendcounts_elems); free(displs_elems);

    MPI_Finalize();
    return 0;
}
