#!/usr/bin/env python3
# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

import argparse
import multiprocessing
import os
import subprocess
import sys

import yaml


def main():
    env = os.environ.copy()

    # Load configuration from config.yml
    with open("../config.yml", "r") as config_file:
        config = yaml.safe_load(config_file)

    # Extract common cmake_args from the YAML config
    cmake_args = config["common"]["cmake_args"]

    # Command-line argument parsing
    parser = argparse.ArgumentParser(description="Build nimblenet script.")
    parser.add_argument("-s", "--simulator", action="store_true", required=False, help="Enable Simulation Mode.")
    parser.add_argument("-p", "--testing", action="store_true", help="Enable Testing Mode only for CI build, for local add argument in config.yaml common cmake_args.")
    parser.add_argument("-c", "--ci_build", action="store_true", required=False, help="Build simulator whl and install in local environment.")
    parser.add_argument("-g", "--coverage", action="store_true", required=False, help="Generate coverage data.")
    parser.add_argument(
        "--doc", action="store_true", required=False, help="Build documentation only."
    )

    args = parser.parse_args()

    # Determine architecture
    arch = subprocess.check_output(['uname', '-p'], text=True).strip()
    if arch == "unknown":
        arch = subprocess.check_output(['uname', '-m'], text=True).strip()
    jobs = multiprocessing.cpu_count()
    python_version = "3.10"  # default python version for simulator
    STRIP = 0
    if args.simulator:
        python_version_major = sys.version_info.major
        python_version_minor = sys.version_info.minor
        python_version = f"{python_version_major}.{python_version_minor}"

        cmake_args += " " + config["simulator"]["cmake_args"] + " "
        cmake_args += f" -DSIMULATION_MODE=1 -DPYTHON_VERSION={python_version}"
        if "-DCMAKE_BUILD_TYPE=Release" in cmake_args:
            STRIP = 1

    CMAKE_CXX_FLAGS = ""
    if args.testing:
        cmake_args += " -DTESTING=1 "

    if args.coverage:
        CMAKE_CXX_FLAGS += " --coverage "

    COMMON_FLAGS = (
        f"-B{os.getcwd()}/build/ "
        f"{cmake_args} "
    )

    # Determine compiler settings based on architecture
    if arch == "arm":
        cmake_command = f"cmake CMakeLists.txt {COMMON_FLAGS} -DCMAKE_CXX_COMPILER=g++ -DMACOS=1 -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_CXX_FLAGS='{CMAKE_CXX_FLAGS}'"
    elif arch == "x86_64":
        CMAKE_CXX_FLAGS += " -stdlib=libstdc++ "
        cmake_command = (
            f"cmake CMakeLists.txt {COMMON_FLAGS} "
            f"-DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS='{CMAKE_CXX_FLAGS}'"
        )
    else:
        cmake_command = f"cmake CMakeLists.txt {COMMON_FLAGS} -DMACOS=1"

    print("cmake command used for compiling: ", cmake_command)

    subprocess.run(cmake_command, shell=True, check=True, env=env)

    # Build documentation
    if args.doc:
        # TODO: Generate Doxygen based documentation as well.
        subprocess.run(
            ["cmake", "--build", "build", "--target", "delitepy_docs"],
            check=True,
        )
        sys.exit()

    # Build
    os.chdir("build")
    subprocess.run(f"make -j{jobs}", shell=True, check=True, env=env)

    # Install or Strip
    if STRIP:
        subprocess.run(f"make -j{jobs} install/strip", shell=True, check=True)
    else:
        subprocess.run(f"make -j{jobs} install", shell=True, check=True)
    os.chdir("..")

    # Simulation Mode
    if args.simulator:
        if not args.ci_build:
            # re-install deliteai
            subprocess.run(f"python{python_version} -m pip uninstall deliteai", shell=True, check=True)
            subprocess.run("rm -rf dist deliteai*", shell=True, check=True)
            subprocess.run(f"python{python_version} setup.py bdist_wheel", shell=True, check=True)
            subprocess.run(f"python{python_version} -m pip install dist/*", shell=True, check=True)

            # re-install delitepy-library-stubs
            subprocess.run(
                f"python{python_version} -m pip uninstall delitepy-library-stubs",
                shell=True,
                check=True,
            )
            subprocess.run(
                f"python{python_version} -m pip install .",
                shell=True,
                cwd="delitepy/library_stubs",
                check=True,
            )


if __name__ == "__main__":
    main()
