# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

# Test streaming JSON APIs

from delitepy import nimblenet as nm
from delitepy import nimblenetInternalTesting as nmi


internal_char_stream = None
json_stream = None
items_stream_iter = None


def create_json_stream(inp):
    internal_char_stream = nmi.create_simulated_char_stream(
        '{"description": "this is a description", "count": 19, "items": ["alpha", "beta", "gamma", "omega"]}',
        15,  # characters per second pushed in json_stream
        10,  # size of buffer between model thread and script thread
    )
    json_stream = internal_char_stream.skip_text_and_get_json_stream()

    # Wait for response to be generated till items key is also generated
    items = json_stream.get_blocking("items")
    items_stream_iter = items.iterator()

    return {}


def get_description(inp):
    return {"description": json_stream.get_blocking_str("description")}


def get_count(inp):
    return {"count": json_stream.get_blocking_str("count")}


def get_next_item(inp):
    next_stream = items_stream_iter.next_blocking()
    if next_stream:
        next_stream.wait_for_completion()
        return {"item": str(next_stream)}
    return {"item": ""}
