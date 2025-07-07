# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm

add_model = nm.Model("add_model")
multiply_model = nm.Model("multiply_two_model")

def add_and_multiply(input):
    print(input)
    add_model_output = add_model.run(input['num'])
    multiply_model_output = multiply_model.run(add_model_output[0])
    print(multiply_model_output)
    return {'output': multiply_model_output[0]}
