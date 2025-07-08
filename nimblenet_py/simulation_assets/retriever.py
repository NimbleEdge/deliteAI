# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

# Test streaming JSON APIs

from delitepy import nimblenet as nm
from delitepy import nimblenetInternalTesting as nmi

def min(a,b):
    if a<b:
        return a
    return b

internal_char_stream = None
stream = None
items_stream_iter = None
retriever=nm.Retriever("GroceryRAG")


def run_llm(inp):
    internal_char_stream = nmi.create_simulated_char_stream(
        '{"description": "this is a description", "items": ["milk", "paneer", "noodles", "eggs"]}',
        15,  # characters per second pushed in stream
        10,  # size of buffer between model thread and script thread
    )
    stream = internal_char_stream.skip_text_and_get_json_stream()
    return {}


def get_description(inp):
    return {"description": stream.get_blocking_str("description")}

def get_next_item(inp):
    if not items_stream_iter:
        # Wait for response to be generated till items key is also generated
        items = stream.get_blocking("items")
        items_stream_iter = items.iterator()

    next_stream = items_stream_iter.next_blocking()
    if next_stream:
        next_stream.wait_for_completion()
        scores, docs = retriever.topk(str(next_stream),1)
        if scores[0] > 0.4:
            return {"item": docs[0], "relevance_score": scores[0]}
        else:
            get_next_item({})
    return {"item": ""}
