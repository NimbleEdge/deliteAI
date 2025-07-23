#!/usr/bin/env python3
#-*- coding: utf-8 -*-

"""
Enhanced Qwen3-1.7B ONNX Demo with Tool Calling

This demo uses a custom enhanced ONNX model with:
- Integrated ArgMax for token generation
- Built-in EOS detection
- Temperature scaling for language confusion mitigation
- Automatic cache management
- English-only output filtering

The enhanced model is created by export.py and saved as model_enhanced.onnx
"""

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
from jinja2 import Environment
import re

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
Always respond directly and call the appropriate tool when needed.
"""

initial_message_block = [
    {
        "role": "system",
        "content": INITIAL_PROMPT
    }
]

config = AutoConfig.from_pretrained(model_id)
tokenizer = Tokenizer.from_pretrained(model_id)
chat_template = "{%- if tools %}\n    {{- '<|im_start|>system\\n' }}\n    {%- if messages[0].role == 'system' %}\n        {{- messages[0].content + '\\n\\n' }}\n    {%- endif %}\n    {{- \"# Tools\\n\\nYou may call one or more functions to assist with the user query.\\n\\nYou are provided with function signatures within <tools></tools> XML tags:\\n<tools>\" }}\n    {%- for tool in tools %}\n        {{- \"\\n\" }}\n        {{- tool | tojson }}\n    {%- endfor %}\n    {{- \"\\n</tools>\\n\\nFor each function call, return a json object with function name and arguments within <tool_call></tool_call> XML tags:\\n<tool_call>\\n{\\\"name\\\": <function-name>, \\\"arguments\\\": <args-json-object>}\\n</tool_call><|im_end|>\\n\" }}\n{%- else %}\n    {%- if messages[0].role == 'system' %}\n        {{- '<|im_start|>system\\n' + messages[0].content + '<|im_end|>\\n' }}\n    {%- endif %}\n{%- endif %}\n{%- set ns = namespace(multi_step_tool=true, last_query_index=messages|length - 1) %}\n{%- for message in messages[::-1] %}\n    {%- set index = (messages|length - 1) - loop.index0 %}\n    {%- if ns.multi_step_tool and message.role == \"user\" and not(message.content.startswith('<tool_response>') and message.content.endswith('</tool_response>')) %}\n        {%- set ns.multi_step_tool = false %}\n        {%- set ns.last_query_index = index %}\n    {%- endif %}\n{%- endfor %}\n{%- for message in messages %}\n    {%- if (message.role == \"user\") or (message.role == \"system\" and not loop.first) %}\n        {{- '<|im_start|>' + message.role + '\\n' + message.content + '<|im_end|>' + '\\n' }}\n    {%- elif message.role == \"assistant\" %}\n        {%- set content = message.content %}\n        {%- set reasoning_content = '' %}\n        {%- if message.reasoning_content is defined and message.reasoning_content is not none %}\n            {%- set reasoning_content = message.reasoning_content %}\n        {%- else %}\n            {%- if '</think>' in message.content %}\n                {%- set content = message.content.split('</think>')[-1].lstrip('\\n') %}\n                {%- set reasoning_content = message.content.split('</think>')[0].rstrip('\\n').split('<think>')[-1].lstrip('\\n') %}\n            {%- endif %}\n        {%- endif %}\n        {%- if loop.index0 > ns.last_query_index %}\n            {%- if loop.last or (not loop.last and reasoning_content) %}\n                {{- '<|im_start|>' + message.role + '\\n<think>\\n' + reasoning_content.strip('\\n') + '\\n</think>\\n\\n' + content.lstrip('\\n') }}\n            {%- else %}\n                {{- '<|im_start|>' + message.role + '\\n' + content }}\n            {%- endif %}\n        {%- else %}\n            {{- '<|im_start|>' + message.role + '\\n' + content }}\n        {%- endif %}\n        {%- if message.tool_calls %}\n            {%- for tool_call in message.tool_calls %}\n                {%- if (loop.first and content) or (not loop.first) %}\n                    {{- '\\n' }}\n                {%- endif %}\n                {%- if tool_call.function %}\n                    {%- set tool_call = tool_call.function %}\n                {%- endif %}\n                {{- '<tool_call>\\n{\"name\": \"' }}\n                {{- tool_call.name }}\n                {{- '\", \"arguments\": ' }}\n                {%- if tool_call.arguments is string %}\n                    {{- tool_call.arguments }}\n                {%- else %}\n                    {{- tool_call.arguments | tojson }}\n                {%- endif %}\n                {{- '}\\n</tool_call>' }}\n            {%- endfor %}\n        {%- endif %}\n        {{- '<|im_end|>\\n' }}\n    {%- elif message.role == \"tool\" %}\n        {%- if loop.first or (messages[loop.index0 - 1].role != \"tool\") %}\n            {{- '<|im_start|>user' }}\n        {%- endif %}\n        {{- '\\n<tool_response>\\n' }}\n        {{- message.content }}\n        {{- '\\n</tool_response>' }}\n        {%- if loop.last or (messages[loop.index0 + 1].role != \"tool\") %}\n            {{- '<|im_end|>\\n' }}\n        {%- endif %}\n    {%- endif %}\n{%- endfor %}\n{%- if add_generation_prompt %}\n    {{- '<|im_start|>assistant\\n' }}\n    {%- if enable_thinking is defined and enable_thinking is false %}\n        {{- '<think>\\n\\n</think>\\n\\n' }}\n    {%- endif %}\n{%- endif %}"

# Use the enhanced ONNX model created by export.py
model_path = "./data/onnx/model_enhanced.onnx"

if not os.path.exists(model_path):
    print(f"❌ Enhanced model not found at {model_path}")
    print("📝 Please run export.py first to create the enhanced model")
    print("💡 Run: python export.py")
    sys.exit(1)

# Load the enhanced ONNX model with integrated generation capabilities
print(f"🚀 Loading ONNX model from {model_path}...")
session = onnxruntime.InferenceSession(model_path)

print(f"✅ {model_id} model loaded successfully!")
print(f"✅ Model has {len(session.get_inputs())} inputs and {len(session.get_outputs())} outputs")
print(f"🚀 Features: Integrated ArgMax, EOS detection, temperature scaling, automatic cache updates")

# Global variables for conversation state
conversation_state = {
    "kv_cache": None,
    "attention_mask": None,
    "position_ids": None,
    "sequence_length": 0,
    "conversation_history": []
}

# Print model input/output info for debugging
print(f"\n📋 Model Inputs (first 5):")
for inp in session.get_inputs()[:5]:  # Show first 5 to avoid spam
    print(f"  • {inp.name}: {inp.shape}")
if len(session.get_inputs()) > 5:
    print(f"  ... and {len(session.get_inputs()) - 5} more inputs")

print(f"\n📋 Enhanced Model Outputs:")
for out in session.get_outputs()[:5]:
    if not out.name.startswith('updated_past_key_values'):  # Skip cache outputs to reduce spam
        print(f"  • {out.name}: {out.shape}")
if len(session.get_outputs()) > 5:
    print(f"  • ... and {len(session.get_outputs()) - 5} more outputs")


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

def initialize_conversation_state():
    """Initialize KV cache and conversation state once"""
    global conversation_state

    # Set config values
    num_key_value_heads = config.num_key_value_heads
    head_dim = config.hidden_size // config.num_attention_heads
    num_hidden_layers = config.num_hidden_layers
    hidden_size = config.hidden_size
    batch_size = 1  # Single batch for conversation

    # Initialize KV cache
    kv_cache = {}

    # Check if config has layer_types
    if not hasattr(config, 'layer_types'):
        config.layer_types = [
            "full_attention"
            for _ in range(config.num_hidden_layers)
        ]

    for i in range(num_hidden_layers):
        if config.layer_types[i] == 'full_attention':
            for kv in ('key', 'value'):
                # Initialize with small valid tensor for first generation step
                kv_cache[f'past_key_values.{i}.{kv}'] = np.zeros([batch_size, num_key_value_heads, 1, head_dim], dtype=np.float16)
        elif config.layer_types[i] == 'conv':
            kv_cache[f'past_conv.{i}'] = np.zeros([batch_size, hidden_size, config.conv_L_cache], dtype=np.float16)

    # Initialize conversation state
    conversation_state.update({
        "kv_cache": kv_cache,
        "attention_mask": None,
        "position_ids": None,
        "sequence_length": 0,
        "conversation_history": []
    })

    print("✅ Conversation state and KV cache initialized")

def reset_conversation_state():
    """Reset conversation state for a new conversation"""
    global conversation_state
    conversation_state.update({
        "kv_cache": None,
        "attention_mask": None,
        "position_ids": None,
        "sequence_length": 0,
        "conversation_history": []
    })
    print("🔄 Conversation state reset")

def parse_tool_calls_from_response(response_text: str) -> list:
    """Parse tool calls from model response using multiple formats"""
    tool_calls = []

    # Method 2: Look for JSON-style tool calls: <tool_call>{"name": "func", "arguments": {...}}</tool_call>
    # Using [\s\S] instead of re.DOTALL to match any character including newlines
    json_tool_pattern = r'<tool_call>\s*({[\s\S]*?})\s*</tool_call>'
    json_matches = re.findall(json_tool_pattern, response_text)

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
    """Generate text using full conversation processing (simplified approach)"""
    print("---"*10)
    print("Conversation Messages:")
    print(json.dumps(conversation_messages, indent=4))
    print("---"*10)

    # Always process the full conversation - simpler and more reliable
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
    seq_len = input_ids.shape[1]

    # Create position IDs
    position_ids = np.tile(np.arange(0, seq_len), (batch_size, 1))

    # Set config values
    num_key_value_heads = config.num_key_value_heads
    head_dim = config.hidden_size // config.num_attention_heads
    num_hidden_layers = config.num_hidden_layers
    hidden_size = config.hidden_size

    # Initialize fresh KV cache for each generation
    model_inputs = {
        "input_ids": input_ids,
        "attention_mask": attention_mask,
        "position_ids": position_ids
    }

    # Check if config has layer_types
    if not hasattr(config, 'layer_types'):
        config.layer_types = [
            "full_attention"
            for _ in range(config.num_hidden_layers)
        ]

    # Initialize KV cache
    for i in range(num_hidden_layers):
        if config.layer_types[i] == 'full_attention':
            for kv in ('key', 'value'):
                # Initialize with small valid tensor for first generation step
                model_inputs[f'past_key_values.{i}.{kv}'] = np.zeros([batch_size, num_key_value_heads, 1, head_dim], dtype=np.float16)
        elif config.layer_types[i] == 'conv':
            model_inputs[f'past_conv.{i}'] = np.zeros([batch_size, hidden_size, config.conv_L_cache], dtype=np.float16)

    # Enhanced generation loop
    generated_tokens = []

    for i in range(max_new_tokens):
        # Run the enhanced model
        model_outputs = session.run(None, model_inputs)

        # Parse outputs
        output_names = [output.name for output in session.get_outputs()]
        outputs_dict = dict(zip(output_names, model_outputs))

        # Check for EOS
        if bool(outputs_dict['is_eos'][0, 0]):
            break

        generated_tokens.append(int(outputs_dict['next_token_id'][0, 0]))

        # Update inputs for next iteration
        model_inputs["input_ids"] = outputs_dict['next_token_id']
        model_inputs["attention_mask"] = outputs_dict['updated_attention_mask']

        # For subsequent calls, we need only the last position
        next_position_full = outputs_dict['next_position']
        last_position = next_position_full[:, -1:]
        model_inputs["position_ids"] = last_position

        # Update cache using present outputs
        for cache_key in list(model_inputs.keys()):
            if cache_key.startswith('past_key_values.'):
                parts = cache_key.split('.')
                if len(parts) == 3:
                    layer_num = parts[1]
                    kv_type = parts[2]
                    present_key = f"present.{layer_num}.{kv_type}"

                    if present_key in outputs_dict:
                        model_inputs[cache_key] = outputs_dict[present_key]
                    else:
                        print(f"⚠️ Warning: Expected cache output '{present_key}' not found")
            elif cache_key.startswith('past_conv.'):
                present_key = cache_key.replace("past_conv", "present_conv")
                if present_key in outputs_dict:
                    model_inputs[cache_key] = outputs_dict[present_key]

    # Decode generated tokens
    response = ""
    if generated_tokens:
        generated_tokens_array = np.array([generated_tokens], dtype=np.int64)
        response = tokenizer.decode_batch(generated_tokens_array, skip_special_tokens=True)[0]

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
    """Run tool calling demonstration using the enhanced ONNX model"""
    print("=== Qwen3 1.7B Enhanced ONNX Tool Calling Demo ===\n")
    print(f"📦 Model: {model_id} (Enhanced)")
    print(f"🚀 Enhanced Model Path: {model_path}")
    print(f"✨ Features: ArgMax, EOS detection, temperature scaling, automatic cache updates")
    print(f"🔧 Available tools: {list(tools.keys())}")

    demo_prompts = [
        "What's the weather here today?",
        "Calculate 15 * 23",
        "What time is it in JST timezone?",
        "Where am I located?",
        "Get my location and check the weather there"
    ]

    for i, user_prompt in enumerate(demo_prompts, 1):
        print(f"\n🎮 Demo {i}: {user_prompt}")
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
    print("🔧 Enhanced Qwen3-1.7B ONNX Model Demo")
    print("📝 Uses enhanced ONNX model with integrated generation enhancements")
    print("🎯 Features: ArgMax, EOS detection, temperature scaling, automatic cache management")
    print("🚀 Export: Custom enhanced model with language confusion mitigation")
    print("📁 Model location: ./data/onnx/model_enhanced.onnx")
    print("=" * 80)

    # Run the enhanced demo
    run_tool_calling_demo()
