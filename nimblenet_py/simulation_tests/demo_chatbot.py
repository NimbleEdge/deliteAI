# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from time import sleep
from deliteai import simulator
import sys
import os

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "../../coreruntime/tests")))
from utils.download_from_s3 import download_folder_from_s3

context = """
[
  {
    "type": "user",
    "message": "Hello, I am Donald Trump the President of the USA. Who are you?"
  },
  {
    "type": "assistant",
    "message": "I'm DeliteAI assistant. How can I help you today?"
  },
  {
    "type": "user",
    "message": "Can you explain quantum computing?"
  },
  {
    "type": "assistant",
    "message": "Sure! Quantum computing uses quantum bits, or qubits..."
  }
]
"""

query = "Hey there, who are you?"

NE_CONFIG_OFFLINE = """
{
    "debug": true,
    "online": false
}
"""

download_folder_from_s3(
    "deliteai", "build-dependencies/llama-3.2-1B/onnx", "../simulation_assets/llama-3", False
)

modules = [
    {
        "name": "workflow_script",
        "version": "1.0.0",
        "type": "script",
        "location": {"path": "../simulation_assets/chatbot.py"},
    },
    {
        "name": "llama-3",
        "version": "1.0.0",
        "type": "llm",
        "location": {"path": "../simulation_assets/llama-3"},
    },
]


def main():
    assert simulator.initialize(NE_CONFIG_OFFLINE, modules)
    simulator.run_method("set_context", {"context": context})

    while True:
        query = input("prompt: ")
        simulator.run_method("prompt_llm", {"query": query})

        try:
            while True:
                output_map = simulator.run_method("get_next_str", {})

                if "finished" in output_map:
                    break
                out_str = output_map["str"]
                print(out_str, end="", flush=True)
        except KeyboardInterrupt:
            print("\nInterrupted, stopping generation...")
            simulator.run_method("stop_running", {})

        print()

    print("Finished running demo text")


if __name__ == "__main__":
    main()
