#!/usr/bin/env python
import os
import shutil
from pathlib import Path
import subprocess
import sys

HELP_MSG = f"""
HELP:  {sys.argv[0]} <cmd> [flags]

CMD:
  release      - compile library with release optimizations
  debug        - compile library with debug symbols
  test         - run tests
  test release - run tests with release mode

FLAGS:  
  -sanitize    - add sanitizers to the build
"""

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

def run_tests(cc: str, flags: list[str], lib_artifact: str):
    if "test" not in sys.argv:
        return
    
    build_cmd = [cc, *flags, "test.c", lib_artifact, "-o", "raon_test"]
    print(f"BUILDING WITH: {' '.join(build_cmd)}\n\n")
    res_c = subprocess.run(build_cmd)

    cpp = find_cpp_compiler()
    res_cpp = None
    if cpp is not None:
        build_cpp_cmd = [cpp, "test.cpp", lib_artifact, "-o", "raon_test_cpp"]
        res_cpp = subprocess.run(build_cpp_cmd)

    if res_c.returncode == 0:
        subprocess.run("./raon_test")
    if res_cpp and res_cpp.returncode == 0:
        subprocess.run("./raon_test_cpp")


def main() -> int:
    if len(sys.argv) == 1:
        print(HELP_MSG)
        return 0

    cc = find_c_compiler()
    if cc is None:
        print("Failed to find C compiler")
        return 1


    flags = get_flags()

    lib_artifact = build_library(cc, flags)
    if lib_artifact is None:
        print("Error: Failed to compile library.")
        return 1

    run_tests(cc, flags, lib_artifact)

    return 0


if __name__ == "__main__":
    sys.exit(main())
