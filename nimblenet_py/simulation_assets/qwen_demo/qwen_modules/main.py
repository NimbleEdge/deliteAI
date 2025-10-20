#!/usr/bin/env python3
#-*- coding: utf-8 -*-
from delitepy import nimblenet as nm

from tools import get_tool_results
from tools import print_available_tools
from generation_mixin import QwenGenerationMixin

# Print available tools after successful import
print_available_tools()

# Constants only - avoid complex global variable assignments
MODEL_ID = "onnx-community/Qwen3-1.7B-ONNX"
MODEL_NAME = "qwen3_1_7b_onnx"

# Model must be loaded in global scope as required by DeliteAI simulator
qwenModel = nm.Model(MODEL_NAME)
generationMixinQwen = None
print("Model loaded successfully")

INITIAL_PROMPT = """You are a helpful assistant with access to tools. When you need to use a tool, format your response with JSON between <tool_call> and </tool_call> tokens.

Use this exact format: <tool_call>{"name": "function_name", "arguments": {"param": "value"}}</tool_call>
If a tool requires a argument you don't know the value of check if another tool can give you that information and call that tool first.
Always respond directly and call the appropriate tool when needed."""

@concurrent
def handle_multi_step_request(user_prompt, max_steps, generation_mixin, output_stream_callback):
    """Handle requests that may require multiple tool calls and back and forth"""
    step_results = []

    for step in range(max_steps):
        print("\n--- Step " + str(step + 1) + " ---")
        if step == 0:
            generation_mixin.add_message({
                "role": "user",
                "content": user_prompt
            })
        else:
            generation_mixin.add_message({
                "role": "system",
                "content": "Now use the result from the tool calls to answer the user's question. Call another tool if needed."
            })
        # Generate response
        try:
            response = generation_mixin.generate(qwenModel, output_stream_callback)

            # Parse and execute tool calls
            tool_results = get_tool_results(response)
            has_errors = False
            # Add tool results to conversation as function messages
            for tool_result in tool_results:
                if "error" not in tool_result.keys():
                    generation_mixin.add_message(tool_result)
                else:
                    has_errors = True

            prompt = "continuation"
            if step == 0:
                prompt = user_prompt
            # Store step result
            step_result = {
                "step": step + 1,
                "prompt": prompt,
                "response": response,
                "tool_results": tool_results,
                "has_errors": has_errors,
                "conversation_history": generation_mixin.get_history()
            }
            step_results.append(step_result)

            if len(tool_results) == 0 or has_errors or step >= max_steps - 1:
                print("✓ Completed after "+str(step + 1)+" step(s) with "+str(len(tool_results))+" tool call(s) and has_errors = "+str(has_errors))
                break

        except Exception as e:
            print("Error in step "+str(step + 1)+": "+str(e))
            prompt_text = ""
            if step == 0:
                prompt_text = user_prompt
            else:
                prompt_text = "continuation"
            step_results.append({
                "step": step + 1,
                "prompt": prompt_text,
                "error": str(e),
                "response": None,
                "tool_calls": [],
                "tool_results": [],
                "conversation_history": generation_mixin.get_history()
            })
            break

    return step_results

@concurrent
def init_generation_mixin(input):
    generationMixinQwen = QwenGenerationMixin(
        INITIAL_PROMPT,
        input["tokenizer_config"],
        input["generation_config"],
        "float16",
        400,
        1
    )
    return {"success": True}

@concurrent
def prompt_for_tool_calling(input):
    """Run tool calling demonstration with proper variable scope handling"""
    print("=== Qwen3 1.7B Tool Calling Demo ===\n")
    print("Model: "+MODEL_ID)

    try:
        if str(generationMixinQwen) == "None":
            init_generation_mixin(input)
        else:
            generationMixinQwen.reset()

        print("\nPrompt: "+input["prompt"])
        print("--------------------------------")

        step_results = handle_multi_step_request(input["prompt"], 4, generationMixinQwen, input["output_stream_callback"])
        # Show final summary
        print("\nMulti-step Summary:")
        for step_result in step_results:
            step_num = step_result["step"]
            tool_calls = []
            if "tool_calls" in step_result:
                tool_calls = step_result["tool_calls"]
            if tool_calls:
                print("  Step "+str(step_num)+": "+str(len(tool_calls))+" tool call(s)")
                for call in tool_calls:
                    func_name = call["function_name"]
                    print("    ✓ "+func_name)
        print("\n" + "--------------------------------")

        return {
            "success": True,
            "model_loaded": True,
            "results": step_results[-1]["response"]
        }

    except Exception as e:
        print("Error in demo: " + str(e))
        return {
            "success": False,
            "error": str(e),
            "model_loaded": False
        }
