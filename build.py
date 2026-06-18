#!/usr/bin/env python
import os
import shutil
from pathlib import Path
import subprocess
import sys

cflags = ["-Wall", "-Wextra", "-pedantic", "-std=c11"]

files = [
    "./src/parser.c",
    "./src/lexer.c",
    "./src/str_slice.c",
]

libs = ["m"]

sanitizers = [
    "address",
    "undefined",
    "leak",
]


def find_c_compiler() -> str | None:
    cc = shutil.which("cc")
    return cc


def find_cpp_compiler() -> str | None:
    compilers = [
        shutil.which("g++"),
        shutil.which("clang++"),
    ]

    valid_compilers = list(filter(lambda comp: comp is not None, compilers))
    if valid_compilers and len(valid_compilers) > 0:
        return valid_compilers[0]
    return None


def get_flags() -> list[str]:
    flags = [*cflags]
    if "release" in sys.argv:
        flags.append("-O2")
    else:
        if "-sanitize" in sys.argv or "debug" in sys.argv:
            flags.append("-g")

        if "-sanitize" in sys.argv:
            for sanitizer in sanitizers:
                flags.append(f"-fsanitize={sanitizer}")

    for lib in libs:
        flags.append(f"-l{lib}")

    return flags


def build_library(cc: str, flags: list[str]) -> str | None:
    libname = "libraon.a"

    res = subprocess.run([cc, *flags, '-c', *files])
    if res.returncode != 0:
        return None

    objs = list(map(lambda f: str(Path(f).with_suffix(".o").name), files))
    res = subprocess.run(["ar", "rcs", libname, *objs])
    if res.returncode != 0:
        return None

    return libname


def main() -> int:
    cc = find_c_compiler()
    if cc is None:
        print("Failed to find C compiler")
        return 1


    flags = get_flags()

    libname = build_library(cc, flags)
    if libname is None:
        print("Error: Failed to compile library.")
        return 1

    build_cmd = [cc, *flags, "test.c", libname, "-o", "raon_test"]
    print(f"BUILDING WITH: {' '.join(build_cmd)}\n\n")
    subprocess.run(build_cmd)

    cpp = find_cpp_compiler()
    if cpp is not None:
        build_cpp_cmd = [cpp, "test.cpp", libname, "-o", "raon_test_cpp"]
        subprocess.run(build_cpp_cmd)

    return 0


if __name__ == "__main__":
    sys.exit(main())
