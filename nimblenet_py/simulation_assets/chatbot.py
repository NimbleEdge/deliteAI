# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm
from delitepy import ne_re as re

llm = nm.llm({"name": "llama-3"})
text_stream = None

SYSTEM_PROMPT = """<|start_header_id|>system<|end_header_id|>You are DeliteAI, a chatbot running on device using DeliteAI's platform. Respond to user's questions briefly<|eot_id|>\n"""
USER_PROMPT_BEGIN = "<|start_header_id|>user<|end_header_id|>"
SECTION_END = "<|eot_id|>\n"
ASSISTANT_RESPONSE_BEGIN = "<|start_header_id|>assistant<|end_header_id|>"


def estimate_tokens_by_words(prompt):
    # Count words in the prompt
    word_count = len(re.findall(r"\b\w+\b", prompt))
    # Estimate tokens (1 word ~ 1.33 tokens)
    estimated_tokens = int(word_count * 1.33)
    return estimated_tokens


def clear_prompt(inp):
    llm.cancel()
    return {}


# This function is to test that we can pass FutureDataVariable as parameter
def call_llm_with_prompt(llm, prompt):
    return llm.prompt(prompt)


# System prompt should be set by calling set_context before this function is called
def prompt_llm(inp):
    # Re-init variables
    text_stream = None

    query = inp["query"]
    new_message = USER_PROMPT_BEGIN + query + SECTION_END
    final_prompt = new_message + ASSISTANT_RESPONSE_BEGIN

    text_stream = call_llm_with_prompt(llm, final_prompt)

    return {}


current_response = ""


def stop_running(inp):
    llm.cancel()
    return {}


def get_next_str(inp):
    if text_stream.finished():
        # print("Text stream finished")
        return {"finished": True}

    str = text_stream.next()
    current_response = current_response + str
    return {"str": str}


def set_context(inp):
    context_map = nm.parse_json(inp["context"])
    final_prompt = SYSTEM_PROMPT

    for message_dict in context_map:
        type = message_dict["type"]
        message = message_dict["message"]
        if type == "user":
            final_prompt = final_prompt + USER_PROMPT_BEGIN
        elif type == "assistant":
            final_prompt = final_prompt + ASSISTANT_RESPONSE_BEGIN
        else:
            raise "type should be user or assistant to set context"
        final_prompt = final_prompt + message + SECTION_END

    llm.add_context(final_prompt)

    return {}
