# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm

invalid_dType_model = nm.Model("simple_fp32_to_fp16_add")

def invalid_model_function(input):
    tensor = nm.zeros([1, 1], "float")
    output = invalid_dType_model.run(tensor)

    # Since the model returns fp16 dataType, which we do not support output will be a NoneVariable
    if output:
        return {"output": output[0]}
    return {}
