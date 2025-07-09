# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from deliteai import simulator
import time
import pytest
import sys
import os
import json
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../coreruntime/tests')))
from utils.download_from_s3 import download_folder_from_s3

build_flags = simulator.get_build_flags()

@pytest.mark.skipif("GENAI" not in build_flags, reason = "Need GENAI build flag")
def test_llm():
    
    download_folder_from_s3("deliteai", "build-dependencies/llama-3.2-1B/onnx", "../simulation_assets/llama-3", False)

    modules = [
        {
            "name": "workflow_script",
            "version": "1.0.0",
            "type": "script",
            "location": {
                "path": "../simulation_assets/chatbot.py"
            }
        },
        {
            "name": "llama-3",
            "version": "1.0.0",
            "location": {
                "path": "../simulation_assets/llama-3"
            },
            "type": "llm"
        }
    ]

    assert simulator.initialize("""{"debug": true, "online": false}""", modules)

    while not simulator.is_ready():
        time.sleep(1)

    
    simulator.run_method("prompt_llm", {"query": "How are you?"})

    i = 0
    outputFromLLM = ""
    while(i<5):
        time.sleep(1)
        i += 1
        output_map = simulator.run_method("get_next_str", {})
        if "finished" in output_map:
            break
        outputFromLLM += output_map["str"]

    simulator.run_method("stop_running", {})

    assert len(outputFromLLM) > 0

@pytest.mark.skipif("GENAI" not in build_flags, reason = "Need GENAI build flag")
def test_list_compatible_llms():
    
    download_folder_from_s3("deliteai", "build-dependencies/llama-3.2-1B/onnx", "../simulation_assets/llama-3", False)

    modules = [
        {
            "name": "workflow_script",
            "version": "1.0.0",
            "type": "script",
            "location": {
                "path": "../simulation_assets/list_compatible_llms.py"
            }
        },
        {
            "name": "llama-3",
            "version": "1.0.0",
            "location": {
                "path": "../simulation_assets/llama-3"
            },
            "type": "llm"
        },
        {
            "name": "MobileBenchmarks",
            "version": "1.0.0",
            "type": "document",
            "location": {
                "path": "../simulation_assets/mobile_benchmarks.json"
            }
        }
    ]
    assert simulator.initialize("""{"debug": true, "online": false}""", modules)

    while not simulator.is_ready():
        time.sleep(1)

    
    output = simulator.run_method("get_compatible_llms", {})
    expectedOutput = {'llms': [{'name': 'llama-3', 'provider': 'custom'}]}
    assert expectedOutput == output


def test_invalid_llm():
    download_folder_from_s3("deliteai", "build-dependencies/llama-3.2-1B/onnx", "../simulation_assets/llama-3", False)

    # Rename genai_config.json file so that llm load fails
    os.rename("../simulation_assets/llama-3/genai_config.json", "../simulation_assets/llama-3/genai_config_1.json")
    modules = [
        {
            "name": "workflow_script",
            "version": "1.0.0",
            "type": "script",
            "location": {
                "path": "../simulation_assets/chatbot.py"
            }
        },
        {
            "name": "llama-3",
            "version": "1.0.0",
            "location": {
                "path": "../simulation_assets/llama-3"
            },
            "type": "llm"
        }
    ]

    assert simulator.initialize("""{"debug": true, "online": false}""", modules)

    # Wait for sometime for is_ready, it should be false
    i = 3
    while i >= 0:
        time.sleep(1)
        assert simulator.is_ready() == False
        i = i - 1
    
    try:
        simulator.run_method("prompt_llm", {"query": "How are you?"})
    except RuntimeError as err:
        print(repr(err))
        assert repr(err) == "RuntimeError('Cannot run method prompt_llm since NimbleEdge is not ready\\nError running workflow script.')"

    # Revert back the renaming
    os.rename("../simulation_assets/llama-3/genai_config_1.json", "../simulation_assets/llama-3/genai_config.json")

if __name__ == "__main__":
    test_llm()
    # test_invalid_llm()
    test_list_compatible_llms()
