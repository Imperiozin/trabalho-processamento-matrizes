# trabalho-processamento-matrizes

## BASE

- gcc base.c -o base.exe
- ./base.exe
{--l ou --length : Tamanho da matrix}
{--s ou --seed : a seed que deverá gerar}
{--mt ou --matrix_type : Tipo de geração da matrix; 0 = usando o srand com a seed; 1 =  usando arquivos com nome matriz1.csv e matriz2.csv}
-exemplo: .\base.exe --l 2000 --s 45

## OMP
- gcc -O3 -fopenmp -march=native omp.c -o omp.exe
- .\omp.exe 
{--l ou --length : Tamanho da matrix}
{--s ou --seed : a seed que deverá gerar}
{--mt ou --matrix_type : Tipo de geração da matrix; 0 = usando o srand com a seed; 1 =  usando arquivos com nome matriz1.csv e matriz2.csv}

-exemplo: .\omp.exe --l 2000 --s 40

## CUDA
Precisa usar o terminal x64 Native Tools Command Prompt for VS 2022
- nvcc -O3 cuda.cu -o cuda
- .\cuda.exe
- {--l ou --length : Tamanho da matrix}
{--s ou --seed : a seed que deverá gerar}

## Testes
Na pasta do Tests execute:
python compare.py --exe1 base.exe --exe2 omp.exe --length 2000
ou python compare.py --help

## Artigo
Exemplo: https://sol.sbc.org.br/index.php/sbcup/article/view/3310

## Máquinas

### PC1

### PC2
- AMD Ryzen 9 7900X 12 Core - Cache | L1 12x32K + 12x32K | L2 12x1M | L3 2x32M
- GIGABYTE GeForce RTX 4060 EAGLE OC 8G

Parâmetros:
OMP TILE: 128
CUDA BLOCK: 32

TEMPOS
Tamanho da Matriz   |    1K    |    2K    |    3K    |    4K
OMP                 | 0.00866s | 0.04733s | 0.15866s | 0.32700s
CUDA                | 0.00211s | 0.01419s | 0.05171s | 0.11452s

### PC3
- Intel® Core™ i5-10210U CPU @ 1.60GHz - Cache | L1 256KB | L2 1MB | L3 6MB
- Intel(R) UHD Graphics Family

Parâmetros:
OMP TILE: 128

TEMPOS
Tamanho da Matriz   |    1K    |    2K    |    3K    |    4K
OMP                 | 0.06000s | 0.33800s | 0.96400s | 2.18600s