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