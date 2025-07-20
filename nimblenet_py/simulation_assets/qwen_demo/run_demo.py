#!/usr/bin/env python3
"""
Main driver script for running Qwen demo with tool calling
"""

import sys
sys.path.append('../../../')

from deliteai import simulator
import json

def main():
    """Run the Qwen demo"""
    print("=== Running Qwen Demo ===")
    print("This demo shows Qwen model and tool calling capabilities\n")

    model_name = "qwen3-1.7b"
    vocab_file = "./qwen/vocab.json"
    merges_file = "./qwen/merges.txt"
    config_file = "./qwen/config.json"
    tokenizer_config_file = "./qwen/tokenizer.json"

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
            "path": "./qwen/onnx/model_q4f16.onnx"
        }
    })
    print(f"Added model: {model_name}")


    with open(vocab_file, encoding="utf-8") as vocab_handle:
        vocab = json.load(vocab_handle)

    bpe_merges = []
    with open(merges_file, encoding="utf-8") as merges_handle:
        i = 0
        for line in merges_handle:
            line = line.strip()
            if (i == 0 and line.startswith("#version:")) or not line:
                i = i + 1
                continue
            bpe_merges.append(tuple(line.split()))
            i = i + 1

    with open(config_file, encoding="utf-8") as config_handle:
        config_dict = json.load(config_handle)

    with open(tokenizer_config_file, encoding="utf-8") as tokenizer_config_handle:
        tokenizer_config_dict = json.load(tokenizer_config_handle)

    # Initialize simulator
    print("\nInitializing simulator...")
    config = {"debug": True, "online": False}

    # Initialize with modules
    if not simulator.initialize(json.dumps(config), modules):
        print("Failed to initialize simulator")
        return

    print("Simulator initialized successfully")

    # Run the main function
    print("\nRunning Qwen workflow...\n")
    result = simulator.run_method(
        "run_tool_calling_demo",
        {
            "vocab": vocab,
            "merges": bpe_merges,
            "config_dict": config_dict,
            "tokenizer_config_dict": tokenizer_config_dict,
            "model_name": model_name
        }
    )

    print("\n=== Demo Complete ===")
    if result.get("success"):
        print("✅ Demo completed successfully!")
        if result.get("model_loaded"):
            print("   Model was loaded and inference attempted")
        else:
            print("   Tool demonstrations completed")
    else:
        print(f"❌ Demo failed: {result.get('error', 'Unknown error')}")


if __name__ == "__main__":
    main()
