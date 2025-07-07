# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm

def no_model(input):
    return {"output": "no_model_executed in bad script"}

var = nm.non_existing_function()
