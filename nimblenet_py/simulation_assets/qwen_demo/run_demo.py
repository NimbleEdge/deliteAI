#!/usr/bin/env python3
"""
Main driver script for running Qwen demo with tool calling
"""

import sys
sys.path.append('../../../')

from deliteai import simulator
import json
import time

def main():
    """Run the Qwen demo"""
    print("=== Running Qwen Demo ===")
    print("This demo shows Qwen model and tool calling capabilities\n")

    base_dir = "../../../models/Qwen3-1.7B/data"
    model_name = "qwen3_1_7b_onnx"
    config_file = base_dir+"/config.json"
    tokenizer_config_file = base_dir+"/tokenizer.json"

    # Module configuration for simulator
    modules = [
        {
            "name": "qwen_modules",
            "version": "1.0.0",
            "type": "script",
            "location": {
                "path": "./qwen_modules.zip"
            }
        }
    ]

    # Add model if requested

    modules.append({
        "name": model_name,
        "version": "1.0.0",
        "type": "model",
        "location": {
            "path": base_dir+"/onnx/model_enhanced.onnx"
        }
    })
    print(f"Added model: {model_name}")

    # Initialize simulator
    print("\nInitializing simulator...")
    config =  {"online": False, "debug": True}

    # Initialize with modules
    if not simulator.initialize(json.dumps(config), modules):
        print("Failed to initialize simulator")
        return
    while not simulator.is_ready():
        time.sleep(1)
    print("Simulator initialized successfully")

    with open(tokenizer_config_file, "r") as f:
        tokenizer_config = json.load(f)
    with open(config_file, "r") as f:
        config = json.load(f)
    # Run the main function
    print("\nRunning Qwen workflow...\n")
    result = simulator.run_method("init_generation_mixin", {
        "tokenizer_config": tokenizer_config,
        "generation_config": config,
    })
    print(result)

    def output_stream_callback(input):
        print(input["token_stream"])
        return {"success": True}

    result = simulator.run_method(
        "prompt_for_tool_calling", {
            "prompt": "How is the weather here?",
            "output_stream_callback": output_stream_callback
        }
    )
    print("\n=== Demo Complete ===")

if __name__ == "__main__":
    main()
