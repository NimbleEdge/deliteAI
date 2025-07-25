from delitepy import nimblenet as nm
from delitepy import ne_re as re
from delitepy import tokenizers
from tools import get_tool_schema


def render_jinja_template(messages, tool_dict, add_generation_prompt, enable_thinking):
    """Render the chat template using hardcoded string structure"""
    result = ""
    content_messages = messages
    # If we have tools, build the system message with tools
    if len(messages) > 0 and (messages[0]["role"] == "system") and len(tool_dict) > 0:
        result = result + "<|im_start|>system\n" + messages[0]["content"] + "\n\n"

        result = result + "# Tools\n\n"
        result = result + "You may call one or more functions to assist with the user query.\n\n"
        result = result + "You are provided with function signatures within <tools></tools> XML tags:\n"
        result = result + "<tools>\n"

        # Add each tool as JSON
        for t in tool_dict:
            result = result + str(t) + "\n"

        result = result + "</tools>\n\n"
        result = result + "For each function call, return a json object with function name and arguments within <tool_call></tool_call> XML tags:\n"
        result = result + "<tool_call>\n"
        result = result + "{\"name\": <function-name>, \"arguments\": <args-json-object>}\n"
        result = result + "</tool_call><|im_end|>\n"
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

def apply_chat_template(messages, tls, add_generation_prompt, tokenizer, return_dict, last_position):
    """Apply chat template using Jinja2 rendering"""

    # Use Jinja2 template renderer
    text = render_jinja_template(
        messages,
        [tls_dict["function"] for tls_dict in tls],
        add_generation_prompt,
        True
    )
    token_ids = tokenizer.encode(text)
    input_ids = nm.tensor([token_ids], "int64")

    if return_dict:
        attention_mask = nm.tensor([[1 for _ in range(last_position + len(token_ids))]], "int64")
        return {
            "input_ids": input_ids,
            "attention_mask": attention_mask
        }
    else:
        return input_ids


class QwenKVCache:
    def __init__(self, generation_config, dtype, batch_size):
        self.batch_size = batch_size
        self.kv_cache = {}
        self.num_hidden_layers = generation_config["num_hidden_layers"]
        self.num_key_value_heads = generation_config["num_key_value_heads"]
        self.head_dim = int(generation_config["hidden_size"] / generation_config["num_attention_heads"])
        self.hidden_size = generation_config["hidden_size"]
        self.dtype = dtype

        # Initialize KV cache for all layers
        for i in range(self.num_hidden_layers):
            for kv in ('key', 'value'):
                self.kv_cache['past_key_values.'+str(i)+'.'+kv] = nm.zeros([self.batch_size, self.num_key_value_heads, 1, self.head_dim], self.dtype)

    def get(self):
        return self.kv_cache

    def update(self, model_inputs, model_outputs):
        # Update with new model inputs (input_ids, attention_mask, position_ids)
        for key in model_inputs.keys():
            self.kv_cache[key] = model_inputs[key]

        if model_outputs:
            # Update KV cache states from present outputs
            for cache_key in self.kv_cache.keys():
                if 'past_key_values' in cache_key:
                    splits = re.split(r'\.', cache_key)
                    present_key = "present."+splits[1]+"."+splits[2]
                    if present_key in model_outputs.keys():
                        self.kv_cache[cache_key] = model_outputs[present_key]
                    else:
                        print("⚠️ Warning: Expected cache output "+present_key+" not found")

    def clear(self):
        self.kv_cache = {}
        for i in range(self.num_hidden_layers):
            for kv in ('key', 'value'):
                self.kv_cache['past_key_values.'+str(i)+'.'+kv] = nm.zeros([self.batch_size, self.num_key_value_heads, 1, self.head_dim], self.dtype)

class QwenGenerationMixin:
    def __init__(self, initial_prompt, tokenizer_config, generation_config, dtype, max_new_tokens, batch_size):
        self.initial_prompt = initial_prompt
        self.tokenizer = tokenizers.from_json(tokenizer_config)
        self.generation_config = generation_config
        self.eos_token_id = generation_config["eos_token_id"]
        self.max_new_tokens = max_new_tokens
        self.history = [
        {
            "role": "system",
            "content": initial_prompt
        }]
        self.cache_index = 0
        self.last_position = 0
        self.kv_cache = QwenKVCache(generation_config, dtype, batch_size)
        self.current_token_stream = []

    def add_message(self, message):
        self.history.append(message)

    def get_history(self):
        return self.history

    def build_model_inputs(self):
        print("Conversation Messages: " + str(self.history[self.cache_index:]))
        print("--------------------------------")
        tool_list = []
        if self.cache_index == 0:
            tool_list = get_tool_schema()

        inputs = apply_chat_template(
            self.history[self.cache_index:],
            tool_list,
            True,
            self.tokenizer,
            True,
            self.last_position
        )
        model_inputs = {
            "input_ids": inputs['input_ids'],
            "attention_mask": inputs['attention_mask'],
            "position_ids": nm.tensor([[i+self.last_position for i in range(inputs['input_ids'].shape()[1])]], "int64")
        }
        self.kv_cache.update(model_inputs, None)

    def get_model_inputs(self):
        return self.kv_cache.get()

    def update_cache(self, model_outputs, output_stream_callback):
        # Update like the original demo - use model outputs directly
        next_token_input = {"input_ids": model_outputs["next_token_id"]}

        # Use the model's updated_attention_mask output (linear growth, not exponential)
        next_token_input["attention_mask"] = model_outputs["updated_attention_mask"]

        # Use the model's next_position output
        next_token_input["position_ids"] = model_outputs["next_position"]

        # Update last_position for tracking
        self.last_position = model_outputs["next_position"][0][0]

        self.kv_cache.update(next_token_input, model_outputs)
        self.add_to_token_stream(next_token_input["input_ids"][0][0], output_stream_callback)

    def reset(self):
        self.history = [
        {
            "role": "system",
            "content": self.initial_prompt
        }]
        self.kv_cache.clear()  # This now properly resets past_attention_mask too
        self.cache_index = 0
        self.last_position = 0

    def get_decoded_response(self):
        response = self.tokenizer.decode(nm.tensor(self.current_token_stream, "int32"))
        self.current_token_stream = []
        return response.strip()

    def add_to_token_stream(self, token_id, output_stream_callback):
        self.current_token_stream.append(token_id)
        output_stream_callback({"token_stream": self.tokenizer.decode(nm.tensor([token_id], "int32"))})

    def generate(self, model, output_stream_callback):
        self.build_model_inputs()
        # 3. Generation loop - now with proper attention mask handling
        for iteration in range(self.max_new_tokens):
            model_outputs = model.run(self.get_model_inputs())
            self.update_cache(model_outputs, output_stream_callback)
            # Check for EOS token
            is_eos = model_outputs["is_eos"][0][0]
            if is_eos:
                print("🛑 EOS token detected at iteration " + str(iteration + 1))
                break
        response = self.get_decoded_response()
        self.add_message({
            "role": "assistant",
            "content": response
        })
        print("Model Response: " + str(self.history[-1]))
        self.cache_index = len(self.history)
        return response
