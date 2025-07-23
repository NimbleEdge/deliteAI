#!/usr/bin/env python3
#-*- coding: utf-8 -*-
from delitepy import nimblenet as nm
from delitepy import ne_re as re
from delitepy import tokenizers
from tools import tools_dict
from tools import tool_schema as tls

# Load Qwen3 1.7B 4-bit model and tokenizer
model_id = "onnx-community/Qwen3-1.7B-ONNX"
try:
    qwenModel = nm.Model("qwen3-1.7b")
    print("Model loaded successfully")
except Exception as e:
    print("Error loading model: " + str(e))
    qwenModel = None

chat_template = "{%- if tools %}\n    {{- '<|im_start|>system\\n' }}\n    {%- if messages[0].role == 'system' %}\n        {{- messages[0].content + '\\n\\n' }}\n    {%- endif %}\n    {{- \"# Tools\\n\\nYou may call one or more functions to assist with the user query.\\n\\nYou are provided with function signatures within <tools></tools> XML tags:\\n<tools>\" }}\n    {%- for tool in tools %}\n        {{- \"\\n\" }}\n        {{- tool | tojson }}\n    {%- endfor %}\n    {{- \"\\n</tools>\\n\\nFor each function call, return a json object with function name and arguments within <tool_call></tool_call> XML tags:\\n<tool_call>\\n{\\\"name\\\": <function-name>, \\\"arguments\\\": <args-json-object>}\\n</tool_call><|im_end|>\\n\" }}\n{%- else %}\n    {%- if messages[0].role == 'system' %}\n        {{- '<|im_start|>system\\n' + messages[0].content + '<|im_end|>\\n' }}\n    {%- endif %}\n{%- endif %}\n{%- set ns = namespace(multi_step_tool=true, last_query_index=messages|length - 1) %}\n{%- for message in messages[::-1] %}\n    {%- set index = (messages|length - 1) - loop.index0 %}\n    {%- if ns.multi_step_tool and message.role == \"user\" and not(message.content.startswith('<tool_response>') and message.content.endswith('</tool_response>')) %}\n        {%- set ns.multi_step_tool = false %}\n        {%- set ns.last_query_index = index %}\n    {%- endif %}\n{%- endfor %}\n{%- for message in messages %}\n    {%- if (message.role == \"user\") or (message.role == \"system\" and not loop.first) %}\n        {{- '<|im_start|>' + message.role + '\\n' + message.content + '<|im_end|>' + '\\n' }}\n    {%- elif message.role == \"assistant\" %}\n        {%- set content = message.content %}\n        {%- set reasoning_content = '' %}\n        {%- if message.reasoning_content is defined and message.reasoning_content is not none %}\n            {%- set reasoning_content = message.reasoning_content %}\n        {%- else %}\n            {%- if '</think>' in message.content %}\n                {%- set content = message.content.split('</think>')[-1].lstrip('\\n') %}\n                {%- set reasoning_content = message.content.split('</think>')[0].rstrip('\\n').split('<think>')[-1].lstrip('\\n') %}\n            {%- endif %}\n        {%- endif %}\n        {%- if loop.index0 > ns.last_query_index %}\n            {%- if loop.last or (not loop.last and reasoning_content) %}\n                {{- '<|im_start|>' + message.role + '\\n<think>\\n' + reasoning_content.strip('\\n') + '\\n</think>\\n\\n' + content.lstrip('\\n') }}\n            {%- else %}\n                {{- '<|im_start|>' + message.role + '\\n' + content }}\n            {%- endif %}\n        {%- else %}\n            {{- '<|im_start|>' + message.role + '\\n' + content }}\n        {%- endif %}\n        {%- if message.tool_calls %}\n            {%- for tool_call in message.tool_calls %}\n                {%- if (loop.first and content) or (not loop.first) %}\n                    {{- '\\n' }}\n                {%- endif %}\n                {%- if tool_call.function %}\n                    {%- set tool_call = tool_call.function %}\n                {%- endif %}\n                {{- '<tool_call>\\n{\"name\": \"' }}\n                {{- tool_call.name }}\n                {{- '\", \"arguments\": ' }}\n                {%- if tool_call.arguments is string %}\n                    {{- tool_call.arguments }}\n                {%- else %}\n                    {{- tool_call.arguments | tojson }}\n                {%- endif %}\n                {{- '}\\n</tool_call>' }}\n            {%- endfor %}\n        {%- endif %}\n        {{- '<|im_end|>\\n' }}\n    {%- elif message.role == \"tool\" %}\n        {%- if loop.first or (messages[loop.index0 - 1].role != \"tool\") %}\n            {{- '<|im_start|>user' }}\n        {%- endif %}\n        {{- '\\n<tool_response>\\n' }}\n        {{- message.content }}\n        {{- '\\n</tool_response>' }}\n        {%- if loop.last or (messages[loop.index0 + 1].role != \"tool\") %}\n            {{- '<|im_end|>\\n' }}\n        {%- endif %}\n    {%- endif %}\n{%- endfor %}\n{%- if add_generation_prompt %}\n    {{- '<|im_start|>assistant\\n' }}\n    {%- if enable_thinking is defined and enable_thinking is false %}\n        {{- '<think>\\n\\n</think>\\n\\n' }}\n    {%- endif %}\n{%- endif %}"


TOOL_CALL_START_TOKEN = "<tool_call>"
TOOL_CALL_END_TOKEN = "</tool_call>"
TOOL_RESPONSE_START_TOKEN = "<tool_response>"
TOOL_RESPONSE_END_TOKEN = "</tool_response>"
INITIAL_PROMPT = """You are a helpful assistant with access to tools. When you need to use a tool, format your response with JSON between <tool_call> and </tool_call> tokens.

Use this exact format: <tool_call>{"name": "function_name", "arguments": {"param": "value"}}</tool_call>
If a tool requires a argument you don't know the value of check if another tool can give you that information and call that tool first.
Always respond directly and call the appropriate tool when needed."""

def get_initial_message_block():
    return [
    {
        "role": "system",
        "content": INITIAL_PROMPT
    }
]

def execute_function_call(function_name, arguments, tools):
    """Execute a function call and return the result"""
    if function_name not in tools:
        return {"error": "Function "+function_name+" not found"}

    try:
        function = tools[function_name]
        result = {"error": "Function execution failed"}  # Initialize result

        # Handle each function explicitly to avoid ** operator
        if function_name == "get_weather":
            location = ""
            if "location" in arguments:
                location = arguments["location"]
            unit = "celsius"
            if "unit" in arguments:
                unit = arguments["unit"]
            result = function(location, unit)
        elif function_name == "calculate_math":
            expression = ""
            if "expression" in arguments:
                expression = arguments["expression"]
            result = function(expression)
        elif function_name == "get_current_time":
            timezone = "UTC"
            if "timezone" in arguments:
                timezone = arguments["timezone"]
            result = function(timezone)
        elif function_name == "get_current_location":
            result = function()
        else:
            result = {"error": "Unknown function: " + function_name}

        return result
    except Exception as e:
        return {"error": "Error executing "+function_name+": "+str(e)}

def format_tool_response(result):
    """Format tool execution result using token-based format"""
    return TOOL_RESPONSE_START_TOKEN+str(result)+TOOL_RESPONSE_END_TOKEN

def execute_tool_call_with_response(function_name, arguments, tools):
    """Execute a function call and return both result and formatted response"""
    result = execute_function_call(function_name, arguments, tools)
    formatted_response = format_tool_response(result)
    return result, formatted_response

def parse_tool_calls_from_response(response_text, tools):
    """Parse tool calls from model response using multiple formats"""
    tool_calls = []

    # Method 2: Look for JSON-style tool calls: <tool_call>{"name": "func", "arguments": {...}}</tool_call>
    # Simplified regex without backtracking - match everything between tags
    json_tool_pattern = r'<tool_call>([^<]*)</tool_call>'
    for match in re.finditer(json_tool_pattern, response_text):
        try:
            json_str = match.group(1)  # Extract the first capture group (the JSON content)
            print("DEBUG: Captured JSON string: '"+json_str+"'")  # Debug what we captured
            print("DEBUG: JSON string length: "+str(len(json_str)))  # Check length
            tool_data = nm.parse_json(json_str)
            func_name = tool_data["name"]
            arguments = tool_data["arguments"]

            if func_name in tools:
                tool_calls.append({
                    "function_name": func_name,
                    "arguments": arguments
                })
                print("✓ Parsed JSON tool call: "+func_name+"("+str(arguments)+")")
        except:
            print("⚠ Failed to parse JSON tool call: "+json_str)

    return tool_calls

def render_jinja_template(messages, tools, add_generation_prompt, enable_thinking):
    """Render the chat template using hardcoded string structure"""
    result = ""

    # If we have tools, build the system message with tools
    if tools:
        result = result + "<|im_start|>system\n"

        # Check if first message is system message and include its content
        if len(messages) > 0 and messages[0]["role"] == "system":
            result = result + messages[0]["content"] + "\n\n"

        result = result + "# Tools\n\n"
        result = result + "You may call one or more functions to assist with the user query.\n\n"
        result = result + "You are provided with function signatures within <tools></tools> XML tags:\n"
        result = result + "<tools>\n"

        # Add each tool as JSON
        for tool in tools:
            result = result + str(tool) + "\n"

        result = result + "</tools>\n\n"
        result = result + "For each function call, return a json object with function name and arguments within <tool_call></tool_call> XML tags:\n"
        result = result + "<tool_call>\n"
        result = result + "{\"name\": <function-name>, \"arguments\": <args-json-object>}\n"
        result = result + "</tool_call><|im_end|>\n"
    else:
        # No tools, just add system message if present
        if len(messages) > 0 and messages[0]["role"] == "system":
            result = result + "<|im_start|>system\n" + messages[0]["content"] + "<|im_end|>\n"

    # Process messages
    content_messages = messages
    if len(messages) > 0 and messages[0]["role"] == "system":
        content_messages = messages[1:]

    for i in range(len(content_messages)):
        message = content_messages[i]
        role = message["role"]
        content = message["content"]
        result = result + "<|im_start|>" + role + "\n" + content + "<|im_end|>\n"

    # Add generation prompt if requested
    if add_generation_prompt:
        result = result + "<|im_start|>assistant\n"
        if not enable_thinking:
            result = result + "<think>\n\n</think>\n\n"

    return result

def apply_chat_template(messages, tool_schema, add_generation_prompt, tokenizer, return_dict):
    """Apply chat template using Jinja2 rendering"""

    # Use Jinja2 template renderer
    text = render_jinja_template(
        messages,
        [tool["function"] for tool in tool_schema],
        add_generation_prompt,
        True
    )
    token_ids = tokenizer.encode(text)
    input_ids = nm.tensor([token_ids], "int64")

    if return_dict:
        attention_mask = nm.tensor([[1 for _ in range(len(token_ids))]], "int64")
        return {
            "input_ids": input_ids,
            "attention_mask": attention_mask
        }
    else:
        return input_ids

def generate_with_model(conversation_messages, max_new_tokens, tool_schema, tokenizer, model_config_dict):
    """Generate text using the loaded model with multi-turn conversation support"""
    # Use chat template with tools for multi-turn conversations
    print("--------------------------------")
    print("Conversation Messages:")
    print(conversation_messages)
    print("--------------------------------")

    # 2. Prepare inputs
    inputs = apply_chat_template(
      conversation_messages,
      tool_schema,
      True,
      tokenizer,
      True,
    )
    input_ids = inputs['input_ids']
    attention_mask = inputs['attention_mask']
    batch_size = input_ids.shape()[0]
    position_ids = nm.tensor([[i for i in range(input_ids.shape()[1])] for _ in range(batch_size)], "int64")

    # Set config values
    num_key_value_heads = model_config_dict["num_key_value_heads"]
    head_dim = int(model_config_dict["hidden_size"] / model_config_dict["num_attention_heads"])
    num_hidden_layers = model_config_dict["num_hidden_layers"]
    eos_token_id = model_config_dict["eos_token_id"]
    hidden_size = model_config_dict["hidden_size"]

    model_inputs = {
        "input_ids": input_ids,
        "attention_mask": attention_mask,
        "position_ids": position_ids
    }
    # Initialize past cache values with correct shapes for ONNX model
    if "layer_types" not in model_config_dict:
        model_config_dict["layer_types"] = [
                "full_attention"
                for i in range(model_config_dict["num_hidden_layers"])
            ]
    for i in range(num_hidden_layers):
        if model_config_dict["layer_types"][i] == 'full_attention':
            for kv in ('key', 'value'):
                # Initialize with a small valid tensor that will be replaced after first forward pass
                # Using sequence length 1 to avoid dimension 0 issues
                model_inputs['past_key_values.'+str(i)+'.'+kv] = nm.zeros([batch_size, num_key_value_heads, 1, head_dim], "float16")
        elif model_config_dict["layer_types"][i] == 'conv':
            model_inputs['past_conv.'+str(i)] = nm.zeros([batch_size, hidden_size, model_config_dict["conv_L_cache"]], "float16")

    # 3. Generation loop
    generated_tokens = []
    for i in range(max_new_tokens):
        try:
            model_outputs = qwenModel.run(model_inputs)
            # Check for EOS token
            if model_outputs["is_eos"][0][0]:
                break

            # Update values for next generation loop
            model_inputs["input_ids"] = model_outputs["next_token_id"]
            generated_tokens.append(model_inputs["input_ids"][0][0])

            if "updated_attention_mask" in model_outputs:
                model_inputs["attention_mask"] = model_outputs["updated_attention_mask"]
            if "next_position" in model_outputs:
                model_inputs["position_ids"] = model_outputs["next_position"]
        except Exception as gen_error:
            print("Generation error: " + str(gen_error))
            break

        # Update cache using present outputs (present.X.key/value → past_key_values.X.key/value)
        for cache_key in model_inputs.keys():
            if ('past_key_values.' in cache_key):
                # Convert past_key_values.X.key/value to present.X.key/value
                splits = re.split(r'\.', cache_key)
                present_key = "present."+splits[1]+"."+splits[2]
                if present_key in model_outputs.keys():
                    model_inputs[cache_key] = model_outputs[present_key]
                else:
                    print("⚠️ Warning: Expected cache output "+present_key+" not found")
            elif 'past_conv' in cache_key:
                # Handle conv cache if present
                present_key = cache_key.replace("past_conv", "present_conv")
                if present_key in model_outputs.keys():
                    model_inputs[cache_key] = model_outputs[present_key]
    # 4. Output result - decode only the generated tokens
    response = ""
    if generated_tokens:
        response = tokenizer.decode(nm.tensor(generated_tokens, "int32"))
    return response.strip()


def handle_multi_step_request(user_prompt, max_steps, max_new_tokens, tools, tool_schema, tokenizer, model_config_dict):
    """Handle requests that may require multiple tool calls and back and forth"""
    step_results = []
    conversation_messages = []  # Initialize as empty list, not None
    tool_context = {}  # Store results from previous tool calls

    for step in range(max_steps):
        print("\n--- Step " + str(step + 1) + " ---")
        if step == 0:
            conversation_messages = get_initial_message_block()
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
            response = generate_with_model(conversation_messages, max_new_tokens, tool_schema, tokenizer, model_config_dict)
            print("Model Response: "+response)

            # Parse and execute tool calls
            tool_calls = parse_tool_calls_from_response(response, tools)
            tool_results = []

            if tool_calls:
                print("Executing "+str(len(tool_calls))+" tool call(s):")
                for call in tool_calls:
                    func_name = call["function_name"]
                    arguments = call["arguments"]

                    print("  • "+func_name+"("+str(arguments)+")")
                    result, formatted_response = execute_tool_call_with_response(func_name, arguments, tools)

                    # Store important results for future reference
                    if func_name == "get_current_location" and "location" in result:
                        tool_context["location"] = result["location"]

                    tool_results.append({
                        "function": func_name,
                        "arguments": arguments,
                        "result": result
                    })
                    print("    Result: "+str(result))

            # Add assistant response to conversation
            conversation_messages.append({
                "role": "assistant",
                "content": response
            })

            # Add tool results to conversation as function messages
            for tool_result in tool_results:
                if "error" not in tool_result["result"].keys():
                    conversation_messages.append({
                        "role": "system",
                        "content": "The result of the tool " + str(tool_result['function'])+" is: "+TOOL_RESPONSE_START_TOKEN+str(tool_result['result'])+TOOL_RESPONSE_END_TOKEN
                    })
            prompt = "continuation"
            if step == 0:
                prompt = user_prompt
            # Store step result
            step_result = {
                "step": step + 1,
                "prompt": prompt,
                "response": response,
                "tool_calls": tool_calls,
                "tool_results": tool_results,
                "has_errors": False,
                "tool_context": tool_context,
                "conversation_messages": conversation_messages
            }
            step_results.append(step_result)

            # Check if all tool calls were successful
            if step_result["has_errors"]:
                print("⚠ Stopping due to tool execution errors")
                break

            # Simple continuation logic: if no tools were called, we're done
            if len(tool_calls) == 0:
                print("✓ Completed after "+str(step + 1)+" step(s) - no tool calls needed")
                break

            # If we've reached max steps, stop
            if step >= max_steps - 1:
                print("✓ Reached maximum steps ("+str(max_steps)+")")
                break

            # If tools were executed, continue to next step to see if model wants to do more
            print("✓ Step "+str(step + 1)+" completed with "+str(len(tool_calls))+" tool call(s) - continuing...")

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
                "tool_context": tool_context,
                "conversation_messages": conversation_messages
            })
            break

    return step_results

def run_tool_calling_demo(input):
    """Run tool calling demonstration"""
    print("=== Qwen3 1.7B Tool Calling Demo ===\n")
    print("Model: "+model_id)

    try:
        # Ensure tokenizer has necessary tokens
        tokenizer = tokenizers.from_json(input["tokenizer_config_dict"])

        # Get tool names without using list()
        tool_names = []
        for key in tools_dict.keys():
            tool_names.append(key)
        print("Available tools: "+str(tool_names))

        demo_prompts = [
            "What's the weather here today?",
            # "Calculate 15 * 23",
            # "What time is it in JST timezone?",
            # "Where am I located?",
            # "Get my location and check the weather there"
        ]

        all_results = []
        i = 1
        for user_prompt in demo_prompts:
            print("\nDemo "+str(i)+": "+user_prompt)
            print("--------------------------------")
            step_results = handle_multi_step_request(user_prompt, 4, 400, tools_dict, tls, tokenizer, input["config_dict"])
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
            all_results.append({
                "demo_" + str(i): {
                    "prompt": user_prompt,
                    "steps": len(step_results),
                    "successful": len(step_results) > 0
                }
            })
            i = i + 1

        # Return a proper map with results
        return {
            "success": True,
            "model_loaded": True,
            "total_demos": len(demo_prompts),
            "results": all_results
        }

    except Exception as e:
        print("Error in demo: " + str(e))
        return {
            "success": False,
            "error": str(e),
            "model_loaded": False
        }
