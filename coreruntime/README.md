# CoreRuntime

This document provides step-by-step instructions for setting up, building, and testing
the DeliteAI coreruntime and the DeliteAI Python SDK.

## Setup Instructions

### 1. Clone Repository

```sh
git clone git@https://github.com/NimbleEdge/deliteAI.git
```

### 2. Install build dependencies

```sh
./setup.sh --sdk python
```

### 3. Setup Virtual Environment and install requirements

```sh
python3 -m venv <path-to-virtual-env>
source <path-to-virtual-env>/bin/activate
cd $GIT_ROOT/coreruntime
pip3 install -r requirements.txt
```

## Build Instructions

### Build Coreruntime

Navigate to `$GIT_ROOT/coreruntime` and run:

```sh
export GIT_ROOT="$(git rev-parse --show-toplevel)"
./build.py
```

This will compile an executable `build/nimble_client/nimble_client` from `main.cpp`.

### Build python SDK

Pre-requisite:

```sh
python3 -m pip install "pybind11[global]"
```

Run:

```sh
cd $GIT_ROOT/coreruntime
./build.py --simulator
```

## Testing and Coverage

Pre-requisite: Set up [mockserver](../mockserver/README.md).

### Run Coreruntime tests

```sh
cd $GIT_ROOT/coreruntime
rm -rf build
./build.py --testing
cd build
./nimbletest
```

### Run python SDK tests

```sh
cd $GIT_ROOT/coreruntime
./build.py --simulator
cd ../nimblenet_py/simulation_tests/
python3 -m pytest
```

## Get Coverage data

Pre-requisite: For Linux, since we are using Clang for compilation, appropriate gcov needs to be
used. First, identify the clang version being used, using `clang --version`. Then pass this extra
argument in all the above gcov commands
`--gcov-executable="/usr/lib/llvm-{clang version}/bin/llvm-cov gcov"`.

### Run for nimbletest

```sh
cd $GIT_ROOT/coreruntime
rm -rf build
./build.py --testing --coverage
cd build
./nimbletest

cd ..
gcovr --html-nested coverage.html
open coverage.html
```

### Run for simulator tests

```sh
cd $GIT_ROOT/coreruntime
./build.py --simulator --coverage
cd $GIT_ROOT/nimblenet_py/simulation_tests
python3 -m pytest
cd ../..
gcovr --html-nested coverage.html
open coverage.html
```

The above two will generate html files with directory structure.

To get merged data from the above two:
Instead of `gcovr --html-nested coverage.html` do `gcovr --json run-1.json` and
`gcovr --json run-2.json` for nimbletest and simulation_tests respectively and then merge the two
files using `gcovr --json-add-tracefile "run-*.json" --html-details coverage.html`.
This won't generate nested directory structure.
