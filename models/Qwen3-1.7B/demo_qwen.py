#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import json
import re
import sys
import os
from typing import List
from transformers import AutoConfig, AutoTokenizer
from tokenizers import Tokenizer
import onnxruntime
import numpy as np
from huggingface_hub import hf_hub_download
from jinja2 import Template, Environment

# Add parent directory to path to import tools
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools import tools, tool_schema

# Load Qwen3 1.7B 4-bit model and tokenizer
model_id = "onnx-community/Qwen3-1.7B-ONNX"

TOOL_CALL_START_TOKEN = "<tool_call>"
TOOL_CALL_END_TOKEN = "</tool_call>"
TOOL_RESPONSE_START_TOKEN = "<tool_response>"
TOOL_RESPONSE_END_TOKEN = "</tool_response>"
INITIAL_PROMPT = f"""You are a helpful assistant with access to tools. When you need to use a tool, format your response with JSON between {TOOL_CALL_START_TOKEN} and {TOOL_CALL_END_TOKEN} tokens.

Use this exact format: {TOOL_CALL_START_TOKEN}{{"name": "function_name", "arguments": {{"param": "value"}}}}{TOOL_CALL_END_TOKEN}
If a tool requires a argument you don't know the value of check if another tool can give you that information and call that tool first.
Always respond directly and call the appropriate tool when needed."""

initial_message_block = [
    {
        "role": "system",
        "content": INITIAL_PROMPT
    }
]

config = AutoConfig.from_pretrained(model_id)
print(config)
tokenizer = Tokenizer.from_pretrained(model_id)
chat_template = "{%- if tools %}\n    {{- '<|im_start|>system\\n' }}\n    {%- if messages[0].role == 'system' %}\n        {{- messages[0].content + '\\n\\n' }}\n    {%- endif %}\n    {{- \"# Tools\\n\\nYou may call one or more functions to assist with the user query.\\n\\nYou are provided with function signatures within <tools></tools> XML tags:\\n<tools>\" }}\n    {%- for tool in tools %}\n        {{- \"\\n\" }}\n        {{- tool | tojson }}\n    {%- endfor %}\n    {{- \"\\n</tools>\\n\\nFor each function call, return a json object with function name and arguments within <tool_call></tool_call> XML tags:\\n<tool_call>\\n{\\\"name\\\": <function-name>, \\\"arguments\\\": <args-json-object>}\\n</tool_call><|im_end|>\\n\" }}\n{%- else %}\n    {%- if messages[0].role == 'system' %}\n        {{- '<|im_start|>system\\n' + messages[0].content + '<|im_end|>\\n' }}\n    {%- endif %}\n{%- endif %}\n{%- set ns = namespace(multi_step_tool=true, last_query_index=messages|length - 1) %}\n{%- for message in messages[::-1] %}\n    {%- set index = (messages|length - 1) - loop.index0 %}\n    {%- if ns.multi_step_tool and message.role == \"user\" and not(message.content.startswith('<tool_response>') and message.content.endswith('</tool_response>')) %}\n        {%- set ns.multi_step_tool = false %}\n        {%- set ns.last_query_index = index %}\n    {%- endif %}\n{%- endfor %}\n{%- for message in messages %}\n    {%- if (message.role == \"user\") or (message.role == \"system\" and not loop.first) %}\n        {{- '<|im_start|>' + message.role + '\\n' + message.content + '<|im_end|>' + '\\n' }}\n    {%- elif message.role == \"assistant\" %}\n        {%- set content = message.content %}\n        {%- set reasoning_content = '' %}\n        {%- if message.reasoning_content is defined and message.reasoning_content is not none %}\n            {%- set reasoning_content = message.reasoning_content %}\n        {%- else %}\n            {%- if '</think>' in message.content %}\n                {%- set content = message.content.split('</think>')[-1].lstrip('\\n') %}\n                {%- set reasoning_content = message.content.split('</think>')[0].rstrip('\\n').split('<think>')[-1].lstrip('\\n') %}\n            {%- endif %}\n        {%- endif %}\n        {%- if loop.index0 > ns.last_query_index %}\n            {%- if loop.last or (not loop.last and reasoning_content) %}\n                {{- '<|im_start|>' + message.role + '\\n<think>\\n' + reasoning_content.strip('\\n') + '\\n</think>\\n\\n' + content.lstrip('\\n') }}\n            {%- else %}\n                {{- '<|im_start|>' + message.role + '\\n' + content }}\n            {%- endif %}\n        {%- else %}\n            {{- '<|im_start|>' + message.role + '\\n' + content }}\n        {%- endif %}\n        {%- if message.tool_calls %}\n            {%- for tool_call in message.tool_calls %}\n                {%- if (loop.first and content) or (not loop.first) %}\n                    {{- '\\n' }}\n                {%- endif %}\n                {%- if tool_call.function %}\n                    {%- set tool_call = tool_call.function %}\n                {%- endif %}\n                {{- '<tool_call>\\n{\"name\": \"' }}\n                {{- tool_call.name }}\n                {{- '\", \"arguments\": ' }}\n                {%- if tool_call.arguments is string %}\n                    {{- tool_call.arguments }}\n                {%- else %}\n                    {{- tool_call.arguments | tojson }}\n                {%- endif %}\n                {{- '}\\n</tool_call>' }}\n            {%- endfor %}\n        {%- endif %}\n        {{- '<|im_end|>\\n' }}\n    {%- elif message.role == \"tool\" %}\n        {%- if loop.first or (messages[loop.index0 - 1].role != \"tool\") %}\n            {{- '<|im_start|>user' }}\n        {%- endif %}\n        {{- '\\n<tool_response>\\n' }}\n        {{- message.content }}\n        {{- '\\n</tool_response>' }}\n        {%- if loop.last or (messages[loop.index0 + 1].role != \"tool\") %}\n            {{- '<|im_end|>\\n' }}\n        {%- endif %}\n    {%- endif %}\n{%- endfor %}\n{%- if add_generation_prompt %}\n    {{- '<|im_start|>assistant\\n' }}\n    {%- if enable_thinking is defined and enable_thinking is false %}\n        {{- '<think>\\n\\n</think>\\n\\n' }}\n    {%- endif %}\n{%- endif %}"

filename = "model_q4f16.onnx" # Options: model.onnx
model_path = hf_hub_download(repo_id=model_id, filename=f"onnx/{filename}") # Download the graph
# hf_hub_download(repo_id=model_id, filename=f"onnx/{filename}_data") # Download the weights
session = onnxruntime.InferenceSession(model_path)


print(f"✓ {model_id} model loaded successfully!")


def execute_function_call(function_name: str, arguments: dict) -> dict:
    """Execute a function call and return the result"""
    if function_name not in tools:
        return {"error": f"Function {function_name} not found"}

    try:
        function = tools[function_name]  # Direct access to function object
        result = function(**arguments)
        return result
    except Exception as e:
        return {"error": f"Error executing {function_name}: {str(e)}"}

def format_tool_response(result: dict) -> str:
    """Format tool execution result using token-based format"""
    result_json = json.dumps(result)
    return f"<|tool_response_start|>{result_json}<|tool_response_end|>"

def execute_tool_call_with_response(function_name: str, arguments: dict) -> tuple:
    """Execute a function call and return both result and formatted response"""
    result = execute_function_call(function_name, arguments)
    formatted_response = format_tool_response(result)
    return result, formatted_response

def parse_tool_calls_from_response(response_text: str) -> list:
    """Parse tool calls from model response using multiple formats"""
    tool_calls = []

    # Method 2: Look for JSON-style tool calls: <tool_call>{"name": "func", "arguments": {...}}</tool_call>
    json_tool_pattern = r'<tool_call>\s*({.*?})\s*</tool_call>'
    json_matches = re.findall(json_tool_pattern, response_text, re.DOTALL)

    for json_str in json_matches:
        try:
            tool_data = json.loads(json_str)
            func_name = tool_data.get("name")
            arguments = tool_data.get("arguments", {})

            if func_name in tools:
                tool_calls.append({
                    "function_name": func_name,
                    "arguments": arguments
                })
                print(f"✓ Parsed JSON tool call: {func_name}({arguments})")
        except json.JSONDecodeError:
            print(f"⚠ Failed to parse JSON tool call: {json_str}")

    return tool_calls


def render_jinja_template(messages, tools=None, add_generation_prompt=False, enable_thinking=True):
    """Render the chat template using Jinja2"""

    # Create Jinja2 environment
    env = Environment()

    # Add custom filters that might be used in the template
    def tojson(obj):
        return json.dumps(obj)

    env.filters['tojson'] = tojson

    # Parse the template
    template = env.from_string(chat_template)

    # Render the template with the provided data
    rendered = template.render(
        messages=messages,
        tools=tools,
        add_generation_prompt=add_generation_prompt,
        enable_thinking=enable_thinking
    )

    return rendered


def apply_chat_template(messages, tool_schema, add_generation_prompt, tokenize, return_dict):
    """Apply chat template using Jinja2 rendering"""

    # Use Jinja2 template renderer
    text = render_jinja_template(
        messages=messages,
        tools=[tool["function"] for tool in tool_schema],
        add_generation_prompt=add_generation_prompt,
        enable_thinking=True
    )
    print("---"*10)
    print("Rendered Text:")
    print(text)
    print("---"*10)
    if tokenize:
        encoding = tokenizer.encode(text, add_special_tokens=False)
        input_ids = np.array([encoding.ids], dtype=np.int64)

        if return_dict:
            attention_mask = np.ones_like(input_ids, dtype=np.int64)
            return {
                "input_ids": input_ids,
                "attention_mask": attention_mask
            }
        else:
            return input_ids
    else:
        return text

def generate_with_model(conversation_messages: List, max_new_tokens: int = 150) -> str:
    """Generate text using the loaded model with multi-turn conversation support"""
    # Use chat template with tools for multi-turn conversations
    print("---"*10)
    print("Conversation Messages:")
    print(json.dumps(conversation_messages, indent=4))
    print("---"*10)

    # 2. Prepare inputs
    inputs = apply_chat_template(
      conversation_messages,
      tool_schema=tool_schema,
      add_generation_prompt=True,
      tokenize=True,
      return_dict=True,
    )
    input_ids = inputs['input_ids']
    attention_mask = inputs['attention_mask']
    batch_size = input_ids.shape[0]
    position_ids = np.tile(np.arange(0, input_ids.shape[-1]), (batch_size, 1))

    # Set config values
    num_key_value_heads = config.num_key_value_heads
    head_dim = config.hidden_size // config.num_attention_heads
    num_hidden_layers = config.num_hidden_layers
    eos_token_id = config.eos_token_id
    hidden_size = config.hidden_size
    # Initialize past cache values with correct shapes for ONNX model
    past_cache_values = {}

    # Check if config has layer_types (like LFM2)
    if hasattr(config, 'layer_types'):
        for i in range(num_hidden_layers):
            if config.layer_types[i] == 'full_attention':
                for kv in ('key', 'value'):
                    # Use the ONNX model's expected head count (8) from the input shapes
                    past_cache_values[f'past_key_values.{i}.{kv}'] = np.zeros([batch_size, 8, 0, head_dim], dtype=np.float16)
            elif config.layer_types[i] == 'conv':
                past_cache_values[f'past_conv.{i}'] = np.zeros([batch_size, hidden_size, config.conv_L_cache], dtype=np.float16)
    else:
        # Standard transformer layers - use ONNX model's expected head count (8)
        for i in range(num_hidden_layers):
            for kv in ('key', 'value'):
                # Use 8 heads as expected by the ONNX model (from debug output)
                past_cache_values[f'past_key_values.{i}.{kv}'] = np.zeros([batch_size, 8, 0, head_dim], dtype=np.float16)

    # 3. Generation loop
    generated_tokens = []
    for i in range(max_new_tokens):
        logits, *present_cache_values = session.run(None, dict(
            input_ids=input_ids,
            attention_mask=attention_mask,
            position_ids=position_ids,
            **past_cache_values,
        ))

        # Update values for next generation loop
        logits_array = np.asarray(logits)
        next_token_id = np.argmax(logits_array[0, -1, :])

        # Check for EOS token
        if next_token_id == eos_token_id:
            break

        generated_tokens.append(next_token_id)
        input_ids = np.array([[next_token_id]], dtype=np.int64)
        attention_mask = np.concatenate([attention_mask, np.ones_like(input_ids, dtype=np.int64)], axis=-1)
        position_ids = position_ids[:, -1:] + 1

        # Update cache
        for j, key in enumerate(past_cache_values):
            past_cache_values[key] = present_cache_values[j]

    # 4. Output result - decode only the generated tokens
    if generated_tokens:
        generated_tokens_array = np.array([generated_tokens], dtype=np.int64)
        response = tokenizer.decode_batch(generated_tokens_array, skip_special_tokens=True)[0]
    else:
        response = ""
    return response.strip()


def handle_multi_step_request(user_prompt: str, max_steps: int, max_new_tokens: int) -> list:
    """Handle requests that may require multiple tool calls and back and forth"""
    step_results = []
    conversation_messages: List[dict] = []  # Initialize as empty list, not None
    tool_context = {}  # Store results from previous tool calls

    for step in range(max_steps):
        print(f"\n--- Step {step + 1} ---")
        if step == 0:
            conversation_messages = initial_message_block.copy()
            conversation_messages.append({
                "role": "user",
                "content": user_prompt
            })
        else:
            conversation_messages.append({
                "role": "system",
                "content": "Now use the result from the tool calls to answer the user's question. Call another tool if needed."
            })
        # Generate response
        try:
            response = generate_with_model(conversation_messages, max_new_tokens=max_new_tokens)
            print(f"Model Response: {response}")

            # Parse and execute tool calls
            tool_calls = parse_tool_calls_from_response(response)
            tool_results = []

            if tool_calls:
                print(f"Executing {len(tool_calls)} tool call(s):")
                for call in tool_calls:
                    func_name = call["function_name"]
                    arguments = call["arguments"]

                    print(f"  • {func_name}({arguments})")
                    result, formatted_response = execute_tool_call_with_response(func_name, arguments)

                    # Store important results for future reference
                    if func_name == "get_current_location" and "location" in result:
                        tool_context["location"] = result["location"]

                    tool_results.append({
                        "function": func_name,
                        "arguments": arguments,
                        "result": result
                    })
                    print(f"    Result: {json.dumps(result, indent=4)}")

            # Add assistant response to conversation
            conversation_messages.append({
                "role": "assistant",
                "content": response
            })

            # Add tool results to conversation as function messages
            for tool_result in tool_results:
                if not tool_result["result"].get("error"):
                    conversation_messages.append({
                        "role": "system",
                        "content": f"The result of the tool {tool_result['function']} is: {TOOL_RESPONSE_START_TOKEN}{json.dumps(tool_result['result'])}{TOOL_RESPONSE_END_TOKEN}"
                    })

            # Store step result
            step_result = {
                "step": step + 1,
                "prompt": user_prompt if step == 0 else "continuation",
                "response": response,
                "tool_calls": tool_calls,
                "tool_results": tool_results,
                "has_errors": any("error" in result.get("result", {}) for result in tool_results),
                "tool_context": tool_context.copy(),
                "conversation_messages": conversation_messages.copy()
            }
            step_results.append(step_result)

            # Check if all tool calls were successful
            if step_result["has_errors"]:
                print(f"⚠ Stopping due to tool execution errors")
                break

            # Simple continuation logic: if no tools were called, we're done
            if not tool_calls:
                print(f"✓ Completed after {step + 1} step(s) - no tool calls needed")
                break

            # If we've reached max steps, stop
            if step >= max_steps - 1:
                print(f"✓ Reached maximum steps ({max_steps})")
                break

            # If tools were executed, continue to next step to see if model wants to do more
            print(f"✓ Step {step + 1} completed with {len(tool_calls)} tool call(s) - continuing...")

        except Exception as e:
            print(f"Error in step {step + 1}: {e}")
            step_results.append({
                "step": step + 1,
                "prompt": user_prompt if step == 0 else "continuation",
                "error": str(e),
                "response": None,
                "tool_calls": [],
                "tool_results": [],
                "tool_context": tool_context.copy(),
                "conversation_messages": conversation_messages.copy() if conversation_messages else []
            })
            break

    return step_results

def run_tool_calling_demo():
    """Run tool calling demonstration"""
    print("=== Qwen3 1.7B Tool Calling Demo ===\n")
    print(f"Model: {model_id}")
    print(f"Available tools: {list(tools.keys())}")

    demo_prompts = [
        "What's the weather here today?",
        "Calculate 15 * 23",
        "What time is it in JST timezone?",
        "Where am I located?",
        "Get my location and check the weather there"
    ]

    for i, user_prompt in enumerate(demo_prompts, 1):
        print(f"\nDemo {i}: {user_prompt}")
        print("-" * 60)
        step_results = handle_multi_step_request(user_prompt, max_steps=4, max_new_tokens=400)
        # Show final summary
        print(f"\n📋 Multi-step Summary:")
        for step_result in step_results:
            step_num = step_result["step"]
            tool_calls = step_result.get("tool_calls", [])
            if tool_calls:
                print(f"  Step {step_num}: {len(tool_calls)} tool call(s)")
                for call in tool_calls:
                    func_name = call["function_name"]
                    print(f"    ✓ {func_name}")
        print("\n" + "="*60)


if __name__ == "__main__":
    # Run the regular demo first
    run_tool_calling_demo()
