# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm

model = nm.Model("multiply_two_model")

def multiply(input):
    print(input)
    model_output = model.run(input['num'])
    print(model_output)
    return {'output': model_output[0]}
