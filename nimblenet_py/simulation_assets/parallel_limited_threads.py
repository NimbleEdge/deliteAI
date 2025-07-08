# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm
from delitepy import nimblenetInternalTesting as nmi


nm.set_threadpool_threads(1)
executor = nm.ConcurrentExecutor()
map = {}

@concurrent
def func_with_race(index):
    map[str(index)] = index
    return index

@concurrent
def run_fun_with_race_in_parallel(n):
    executor.run_parallel(func_with_race, range(n+1))
    return

def test_parallel_inside_parallel(input):
    executor.run_parallel(run_fun_with_race_in_parallel, range(input["n"]))
    return {"map": map}
