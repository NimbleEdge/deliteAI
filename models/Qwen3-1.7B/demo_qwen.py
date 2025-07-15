#!/usr/bin/env python3
#-*- coding: utf-8 -*-

import json
import datetime
import torch
import re
from typing import Tuple, List, Optional
from transformers import AutoModelForCausalLM, AutoTokenizer
from tools import tools, tool_schema

# Load Qwen3 1.7B 4-bit model and tokenizer
model_id = "Qwen/Qwen3-1.7B"

TOOL_CALL_START_TOKEN = "<tool_call>"
TOOL_CALL_END_TOKEN = "</tool_call>"
TOOL_RESPONSE_START_TOKEN = "<tool_response>"
TOOL_RESPONSE_END_TOKEN = "</tool_response>"
INITIAL_PROMPT = f"""You are a helpful assistant. When you need to use tools, format your response with the tool call between {TOOL_CALL_START_TOKEN} and {TOOL_CALL_END_TOKEN} tokens.
Use this format: {TOOL_CALL_START_TOKEN}[function_name(param="value")]{TOOL_CALL_END_TOKEN}. Call only one tool at a time and sequentially execute them."""

initial_message_block = [
    {
        "role": "system",
        "content": INITIAL_PROMPT
    }
]

from mlx_lm import load, generate

model, tokenizer = load("mlx-community/Qwen3-1.7B-4bit")
# from transformers import BitsAndBytesConfig

# Configure 4-bit quantization
# quantization_config = BitsAndBytesConfig(
#     load_in_4bit=True,
#     bnb_4bit_compute_dtype=torch.bfloat16,
#     bnb_4bit_use_double_quant=True,
#     bnb_4bit_quant_type="nf4"
# )

# model = AutoModelForCausalLM.from_pretrained(
#     model_id,
#     device_map="auto",
#     torch_dtype=torch.bfloat16,
#     # quantization_config=quantization_config,
#     trust_remote_code=True,
# )
# tokenizer = AutoTokenizer.from_pretrained(model_id, trust_remote_code=True)

# Ensure tokenizer has necessary tokens
if tokenizer.pad_token is None:
    tokenizer.pad_token = tokenizer.eos_token
    
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

def generate_with_model(conversation_messages: List, max_new_tokens: int = 150) -> str:
    """Generate text using the loaded model with multi-turn conversation support"""
    # Use chat template with tools for multi-turn conversations
    print("---"*10)
    print("Conversation Messages:")
    print(json.dumps(conversation_messages, indent=4))
    print("---"*10)
    prompt = tokenizer.apply_chat_template(
        conversation_messages,
        tools=tool_schema,
        add_generation_prompt=True,
        tokenize=False
    )
    
    response = generate(model, tokenizer, prompt)
    # Tokenize the prompt
    # input_ids = tokenizer.encode(prompt, return_tensors="pt").to(model.device)
    
    # # Generate response with parameters optimized for tool calling
    # with torch.no_grad():
    #     output = model.generate(
    #         input_ids,
    #         do_sample=True,
    #         temperature=0.3,  # Good balance for Qwen3
    #         top_p=0.8,        # Nucleus sampling for focused responses
    #         max_new_tokens=max_new_tokens,
    #         pad_token_id=tokenizer.pad_token_id or tokenizer.eos_token_id,
    #         eos_token_id=tokenizer.eos_token_id,
    #         repetition_penalty=1.1,  # Prevent repetition
    #     )
    
    # response = tokenizer.decode(output[0][input_ids.shape[1]:], skip_special_tokens=True)
    return response.strip()

def handle_multi_step_request(user_prompt: str, max_steps: int, max_new_tokens: int) -> list:
    """Handle requests that may require multiple tool calls and back and forth"""
    step_results = []
    conversation_messages = None  # Will hold the full conversation chain
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