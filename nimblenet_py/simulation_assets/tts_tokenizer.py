# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm
from delitepy import ne_re as re

space_char = " "
vocab = {}

def init(input):
    vocab = input["vocab"]
    return {}

def preprocess(text, language):
    if language == "en":
        return text.lower()
    else:
        return text.unicode()

def tokenize(input):
    text = input["text"]
    language = input["language"]
    text = preprocess(text, language)
    cs = []
    for c in text:
        # Since the input can have both hindi and english text, doing str(c) for any sort of string computation
        # Add a whitespace if the current char is a whitespace while the previous char is not a whitespace.\
        if str(c) == space_char and len(cs) > 0 and cs[len(cs) - 1] != space_char:
            cs.append(str(c))
        # Add the current char that is an alphanumeric or an apostrophe.
        elif str(c) in vocab:
            cs.append(str(c))
        else:
            continue

    # Remove trailing spaces
    index = -1
    if cs:
        # Find the first character from end which is non whitespace
        for i in range(len(cs)):
            if cs[len(cs) - i - 1] != space_char:
                index = len(cs) - i
                break

    modified_cs = [space_char]
    if index != -1:
        for i in range(index):
            modified_cs.append(cs[i])
    modified_cs.append(space_char)

    tokens = []
    for c in modified_cs:
        tokens.append(vocab[c])
    return {"tokens": tokens}
