# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm

# Test recursive function
def countdown(a):
    if a == 0:
        return 0
    return countdown(a - 1)


# Input to the function is a dictionary
def main(inputs):
    tensorVar = nm.tensor([1.0, 2.0, 3.0, 4.5, 0.1], "double")
    minTensor = nm.min(tensorVar)
    maxTensor = nm.max(tensorVar)
    sumTensor = nm.sum(tensorVar)
    meanTensor = nm.mean(tensorVar)

    # Empty tensor
    emptyTensorOfFloats = nm.tensor([], "float")
    emptyTensorOfStrings = nm.zeros([0], "string")

    listOfStrings = nm.tensor(["a", "b", ":gfdfdsfsdfs"], "string")

    # Take single variables as input
    inputString = inputs["singleString"]
    outputString = ""

    if inputString == "singleString":
        outputString = str(1234)

    outputFloat = inputs["singleFloat"] ** 2

    # Check printing of bool tensor is correct
    boolTensorStr = str(inputs["boolTensor"])

    # Iterate over a string and return list
    sampleString = "Hello there!"
    char_list = []
    for c in sampleString:
        char_list.append(c)

    # The output returned should always be a dictionary
    return {
        "outputSingleString": outputString,
        "outputSingleFloat": outputFloat,
        "listOfStrings": listOfStrings,
        "emptyTensorOfFloats": emptyTensorOfFloats,
        "emptyTensorOfStrings": emptyTensorOfStrings,
        "countdownZero": countdown(5),
        "minTensor": minTensor,
        "maxTensor": maxTensor,
        "sumTensor": sumTensor,
        "meanTensor": meanTensor,
        "boolTensorStr": boolTensorStr,
        "char_list": char_list,
    }
