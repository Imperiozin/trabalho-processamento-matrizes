import subprocess
import os
import sys

def compile_and_run(cfile):
    exe = cfile.replace(".c", ".exe" if os.name == "nt" else "")
    print(f"Compilando {cfile}...")

    # Compilar
    compile_cmd = ["gcc", cfile, "-O3", "-fopenmp", "-march=native", "-pthread" ,"-o", exe]
    comp = subprocess.run(compile_cmd, capture_output=True, text=True)

    if comp.returncode != 0:
        print(f"Erro ao compilar {cfile}:")
        print(comp.stderr)
        return None
    
    print(f"Executando {exe}...")
    # Executar
    run = subprocess.run([exe], capture_output=True, text=True)

    if run.returncode != 0:
        print(f"Erro ao executar {exe}:")
        print(run.stderr)
        return None

    return run.stdout.strip()

def main():
    if len(sys.argv) != 3:
        print("Uso: python compare_c_files.py arquivo1.c arquivo2.c")
        return

    file1 = sys.argv[1]
    file2 = sys.argv[2]

    if not os.path.exists(file1) or not os.path.exists(file2):
        print("ERRO: Um dos arquivos .c não existe.")
        return

    out1 = compile_and_run(file1)
    out2 = compile_and_run(file2)

    if out1 is None or out2 is None:
        print("Erro ao executar arquivos.")
        return

    print("\n======= RESULTADOS =======")
    print(f"{file1}: {out1}")
    print(f"{file2}: {out2}")

    print("\n======= COMPARAÇÃO =======")
    if out1 == out2:
        print("As saídas são IGUAIS.")
    else:
        print("As saídas são DIFERENTES. %s != %s" % (out1, out2))

if __name__ == "__main__":
    main()
