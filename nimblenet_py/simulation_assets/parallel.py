# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm
from delitepy import nimblenetInternalTesting as nmi


nm.set_threadpool_threads(5)
executor = nm.ConcurrentExecutor()
globalTensor = None
total = 0
map = {}

@concurrent
def func_with_race(index,tensor):
    total = total +1
    tensor[index] = index
    map[str(index)] = index
    return index

@concurrent
def update_total(n):
    total = total +n

@concurrent
def func_without_race(index,tensor):
    tensor[index] = index**2
    executor.sync(update_total,1)

def test_parallel(inp):
    n = inp["n"]
    total = 0
    indexTensor = nm.zeros([n],"int64")
    executor.run_parallel(func_with_race,range(n),indexTensor)
    incorrectTotal = total

    total =0
    squareTensor = nm.zeros([n],"int64")
    executor.run_parallel(func_without_race,range(n),squareTensor)
    correctTotal = total

    return {
            "incorrectTotal": incorrectTotal,
            "correctTotal": correctTotal,
            "indexTensor": indexTensor,
            "squareTensor": squareTensor,
            "globalTensor": globalTensor,
            "map": map,
    }

@concurrent
def wait_for_ms(n):
    start = nmi.get_chrono_time()
    end = nmi.get_chrono_time()
    while(end -start < 1e3*n):
        end = nmi.get_chrono_time()

@concurrent
def func_with_throw(index,tensor):
    if index == 10:
        ## A way to throw exception
        tensor[100000] = 1
    tensor[index] = index
    wait_for_ms(1)
    return index

def throw_exception_in_parallel(input):
    n = 100
    globalTensor = nm.zeros([n],"int64")
    executor.run_parallel(func_with_throw,range(n),globalTensor)
    return {}
