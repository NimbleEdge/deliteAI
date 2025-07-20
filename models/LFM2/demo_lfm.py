#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import json
import re
import sys
import os
from typing import List

# Add parent directory to path to import tools
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from tools import tools, tool_schema

from transformers import AutoConfig, AutoTokenizer
import onnxruntime
import numpy as np
from huggingface_hub import hf_hub_download

# 1. Load config, processor, and model
model_id = "onnx-community/LFM2-1.2B-ONNX"


TOOL_CALL_START_TOKEN = "<|tool_call_start|>"
TOOL_CALL_END_TOKEN = "<|tool_call_end|>"
TOOL_RESPONSE_START_TOKEN = "<|tool_response_start|>"
TOOL_RESPONSE_END_TOKEN = "<|tool_response_end|>"
INITIAL_PROMPT = f"""You are a helpful assistant. When you need to use tools, call only one tool at a time and sequentially execute them."""

initial_message_block = [
    {
        "role": "system",
        "content": INITIAL_PROMPT
    }
]

config = AutoConfig.from_pretrained(model_id)
tokenizer = AutoTokenizer.from_pretrained(model_id)
filename = "model.onnx" # Options: "model.onnx", "model_fp16.onnx", "model_q4.onnx", "model_q4f16.onnx"
model_path = hf_hub_download(repo_id=model_id, filename=f"onnx/{filename}") # Download the graph
hf_hub_download(repo_id=model_id, filename=f"onnx/{filename}_data") # Download the weights
session = onnxruntime.InferenceSession(model_path)

## Set config values
num_key_value_heads = config.num_key_value_heads
head_dim = config.hidden_size // config.num_attention_heads
num_hidden_layers = config.num_hidden_layers
eos_token_id = config.eos_token_id
hidden_size = config.hidden_size
conv_L_cache = config.conv_L_cache
layer_types = config.layer_types

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
    return f"{TOOL_RESPONSE_START_TOKEN}{result_json}{TOOL_RESPONSE_END_TOKEN}"

def execute_tool_call_with_response(function_name: str, arguments: dict) -> tuple:
    """Execute a function call and return both result and formatted response"""
    result = execute_function_call(function_name, arguments)
    formatted_response = format_tool_response(result)
    return result, formatted_response

def parse_tool_calls_from_response(response_text: str) -> list:
    """Parse tool calls from model response using multiple formats"""
    tool_calls = []

    # Method 2: Look for JSON-style tool calls: <|tool_call_start|>{"name": "func", "arguments": {...}}<|tool_call_end|>
    json_tool_pattern = r'<\|tool_call_start\|>\s*({.*?})\s*<\|tool_call_end\|>'
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

def generate_with_model(conversation_messages: List, max_new_tokens: int = 150) -> str:
    """Generate text using the loaded model with multi-turn conversation support"""
    # Use chat template with tools for multi-turn conversations
    print("---"*10)
    print("Conversation Messages:")
    print(json.dumps(conversation_messages, indent=4))
    print("---"*10)

    # 2. Prepare inputs
    inputs = tokenizer.apply_chat_template(
      conversation_messages,
      tools=tool_schema,
      add_generation_prompt=True,
      tokenize=True,
      return_dict=True,
      return_tensors="np"
    )
    input_ids = inputs['input_ids']
    attention_mask = inputs['attention_mask']
    batch_size = input_ids.shape[0]
    position_ids = np.tile(np.arange(0, input_ids.shape[-1]), (batch_size, 1))
    past_cache_values = {}
    for i in range(num_hidden_layers):
      if layer_types[i] == 'full_attention':
        for kv in ('key', 'value'):
          past_cache_values[f'past_key_values.{i}.{kv}'] = np.zeros([batch_size, num_key_value_heads, 0, head_dim], dtype=np.float32)
      elif layer_types[i] == 'conv':
        past_cache_values[f'past_conv.{i}'] = np.zeros([batch_size, hidden_size, conv_L_cache], dtype=np.float32)
      else:
        raise ValueError(f"Unsupported layer type: {layer_types[i]}")

    # 3. Generation loop
    generated_tokens = np.array([[]], dtype=np.int64)
    for i in range(max_new_tokens):
      logits, *present_cache_values = session.run(None, dict(
          input_ids=input_ids,
          attention_mask=attention_mask,
          position_ids=position_ids,
          **past_cache_values,
      ))

      ## Update values for next generation loop
      input_ids = logits[:, -1].argmax(-1, keepdims=True)
      attention_mask = np.concatenate([attention_mask, np.ones_like(input_ids, dtype=np.int64)], axis=-1)
      position_ids = position_ids[:, -1:] + 1
      for j, key in enumerate(past_cache_values):
        past_cache_values[key] = present_cache_values[j]
      generated_tokens = np.concatenate([generated_tokens, input_ids], axis=-1)
      if (input_ids == eos_token_id).all():
        break

    # 4. Output result
    response = tokenizer.batch_decode(generated_tokens, skip_special_tokens=True)[0]
    return response.strip()

def handle_multi_step_request(user_prompt: str, max_steps: int, max_new_tokens: int) -> list:
    """Handle requests that may require multiple tool calls and back and forth"""
    step_results = []
    conversation_messages : List[dict] = []  # Will hold the full conversation chain
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