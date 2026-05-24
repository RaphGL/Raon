import shutil
import subprocess
import sys

cflags = ["-Wall", "-Wextra", "-pedantic", "-std=c11"]

files = [
    "./parser.c",
    "./lexer.c",
    "./main.c",
]

libs = ["m"]


def find_compiler() -> str:
    cc = shutil.which("cc")
    if cc is None:
        print("Failed to find C compiler")
        sys.exit(1)
    return cc


def get_flags() -> list[str]:
    flags = [*cflags]
    if "-mode:release" in sys.argv:
        flags.append("-O2")
    else:
        flags.append("-g")

    for lib in libs:
        flags.append(f"-l{lib}")

    return flags


def main():
    cc = find_compiler()
    flags = get_flags()

    build_cmd = ' '.join([cc] + flags + files)
    print(f"BUILDING WITH: {build_cmd}\n\n")
    subprocess.run([cc, *flags, *files])


if __name__ == "__main__":
    main()
