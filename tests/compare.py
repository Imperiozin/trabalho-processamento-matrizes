import subprocess
import os
import sys
import time

def run_exe(exe_path, tamanho_matriz, seed):
    if not os.path.exists(exe_path):
        print(f"Erro: arquivo {exe_path} não existe.")
        return None

    print(f"Executando {exe_path}...")

    inicio = time.time()
    run = subprocess.run(
        [exe_path , str(tamanho_matriz), str(seed)],
        capture_output=True,
        text=True
    )
    fim = time.time()

    if run.returncode != 0:
        print(f"Erro ao executar {exe_path}:")
        print(run.stderr)
        return None

    return (run.stdout.strip(), fim - inicio)


def main():
    if len(sys.argv) != 5:
        print("Uso: python compare_execs.py arquivo1.exe arquivo2.exe tamanho_matriz seed")
        return

    exe1 = sys.argv[1]
    exe2 = sys.argv[2]

    tamanho_matriz = sys.argv[3]
    seed = sys.argv[4]

    print(f"Comparando {exe1} e {exe2} com tamanho de matriz {tamanho_matriz} e seed {seed}.")

    # Executar EXE 1
    (out1, time1) = run_exe(exe1 , tamanho_matriz, seed)

    # Executar EXE 2
    (out2, time2) = run_exe(exe2 , tamanho_matriz, seed)

    if out1 is None or out2 is None:
        print("Erro ao executar arquivos.")
        return

    print("\n======= RESULTADOS =======")
    print(f"{exe1}: {out1} - {time1}s")
    print(f"{exe2}: {out2} - {time2}s")

    print("\n======= COMPARAÇÃO =======")
    if out1 == out2:
        print("As saídas são IGUAIS.")
    else:
        print(f"As saídas são DIFERENTES. {out1} != {out2}")


if __name__ == "__main__":
    main()
