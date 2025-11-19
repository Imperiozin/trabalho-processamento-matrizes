# trabalho-processamento-matrizes

## BASE

- gcc base.c -o base.exe
- ./base.exe

- gcc -O3 -fopenmp -march=native omp.c -o omp.exe
- ./omp.exe

## Testes

Na pasta do Tests execute:
python compare.py base.c omp.c