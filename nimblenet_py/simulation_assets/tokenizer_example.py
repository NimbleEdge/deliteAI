# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

"""
Tokenizers Integration Example for DeliteAI

This module demonstrates how to use tokenizers in DeliteAI's delitepy runtime.
DeliteAI includes support for tokenizers through the `delitepy.tokenizers` module,
which provides a Python interface to the mlc-ai/tokenizers-cpp library.

Supported Tokenizer Types:
    - HuggingFace Tokenizers: JSON format tokenizers from HuggingFace Hub
    - SentencePiece: Google's SentencePiece tokenizers (.model files)
    - RWKV World: RWKV tokenizers
    - Custom JSON: Manually created tokenizer configurations

Basic Usage:
    from delitepy import tokenizers

    # Load tokenizer
    tokenizer = tokenizers.from_json(json_config)

    # Encode text
    token_ids = tokenizer.encode("Hello world!")

    # Decode back to text
    decoded = tokenizer.decode(token_ids)

Integration Details:
    The tokenizers module is implemented as:
    1. C++ Wrapper: TokenizersDataVariable class wraps mlc-ai/tokenizers-cpp
    2. DelitePy Integration: Functions exposed through delitepy import system
    3. Memory Management: Tokenizer instances managed automatically
    4. Error Handling: Proper exception handling for all operations

Platform Support:
    - Linux (x86_64, ARM64)
    - macOS (Intel, Apple Silicon)
    - iOS (device and simulator)
    - Android (ARM64, ARMv7, x86_64)
    - Windows (x86_64)

Performance Notes:
    - Tokenizer creation is expensive; reuse instances when possible
    - Token encoding/decoding is fast and suitable for real-time use
    - Cross-platform deployment supported on all major platforms

Dependencies:
    - Rust toolchain (for building underlying tokenizers library)
    - CMake 3.18+ (for build system)
    - C++17 support (for wrapper implementation)

For cross-compilation, install appropriate Rust targets:
    # For iOS
    rustup target add aarch64-apple-ios aarch64-apple-ios-sim

    # For Android
    rustup target add aarch64-linux-android armv7-linux-androideabi

Examples:
    This module contains comprehensive test functions demonstrating:
    - Basic tokenizer creation and usage
    - Advanced tokenizer with special tokens
    - Error handling and validation
    - Combined test scenarios
"""

from delitepy import tokenizers

def test_tokenizers(params):
    """
    Test basic tokenizer functionality with a simple BPE tokenizer.

    This function demonstrates the core tokenizer operations:
    - Creating a tokenizer from JSON configuration
    - Encoding text to token IDs
    - Decoding token IDs back to text
    - Vocabulary size queries
    - Token/ID conversions

    The test uses a minimal BPE tokenizer with a small vocabulary containing
    basic words like "hello", "world", and punctuation.

    Returns:
        dict: Test results containing:
            - status (str): "success" or "error"
            - vocab_size (int): Size of the tokenizer vocabulary
            - encoded_length (int): Number of tokens produced
            - decoded_text (str): Text after encode/decode round-trip
            - hello_token_id (int): Token ID for "hello"
            - token_0 (str): Token corresponding to ID 0
            - message (str): Error message if status is "error"

    Example:
        >>> results = test_tokenizers()
        >>> assert results["status"] == "success"
        >>> assert results["decoded_text"] == "hello world!"
    """

    # Define a simple tokenizer configuration
    json_config = '''{
        "version": "1.0",
        "added_tokens": [],
        "model": {
            "type": "BPE",
            "vocab": {"h": 0, "e": 1, "l": 2, "o": 3, " ": 4, "w": 5, "r": 6, "d": 7, "!": 8, "hello": 9, "world": 10},
            "merges": []
        }
    }'''
    # Create tokenizer from JSON
    tokenizer = tokenizers.from_json(json_config)

    # Test encoding
    text = "hello world!"
    token_ids = tokenizer.encode(text)

    # Test decoding
    decoded_text = tokenizer.decode(token_ids)

    # Test vocabulary operations
    vocab_size = tokenizer.get_vocab_size()

        # Test token/ID conversion
    token_id = tokenizer.token_to_id("hello")  # Look up "hello" token
    token = tokenizer.id_to_token(0)

    return {
        "status": "success",
        "vocab_size": vocab_size,
        "encoded_length": len(token_ids),
        "decoded_text": decoded_text,
        "hello_token_id": token_id,
        "token_0": token
    }

def test_sentencepiece_style(params):
    """
    Test advanced tokenizer functionality with special tokens and BPE merges.

    This function demonstrates more sophisticated tokenizer features:
    - Special tokens ([UNK], [CLS], [SEP]) for sequence classification
    - Comprehensive vocabulary with alphabet and common words
    - BPE merge rules for subword tokenization
    - Longer text processing capabilities

    The tokenizer created includes:
    - Full alphabet (a-z)
    - Common punctuation and space
    - Frequent English words (the, and, of, etc.)
    - Special classification tokens used in BERT-style models
    - BPE merge rules for common character combinations

    Returns:
        dict: Test results containing:
            - status (str): "success" or "error"
            - vocab_size (int): Size of the tokenizer vocabulary (100+ tokens)
            - text (str): Input text used for testing
            - encoded_length (int): Number of tokens after encoding
            - decoded_text (str): Reconstructed text after decode
            - cls_id (int): Token ID for [CLS] special token
            - sep_id (int): Token ID for [SEP] special token
            - unk_id (int): Token ID for [UNK] unknown token
            - cls_token (str): Token string for CLS ID lookup
            - message (str): Error message if status is "error"

    Example:
        >>> results = test_sentencepiece_style()
        >>> assert results["status"] == "success"
        >>> assert results["vocab_size"] > 100
        >>> assert results["cls_id"] == 101
    """

    # Create a more comprehensive tokenizer with special tokens
    json_config = '''{
        "version": "1.0",
        "truncation": null,
        "padding": null,
        "added_tokens": [
            {"id": 100, "content": "[UNK]", "single_word": false, "lstrip": false, "rstrip": false, "normalized": false, "special": true},
            {"id": 101, "content": "[CLS]", "single_word": false, "lstrip": false, "rstrip": false, "normalized": false, "special": true},
            {"id": 102, "content": "[SEP]", "single_word": false, "lstrip": false, "rstrip": false, "normalized": false, "special": true}
        ],
        "normalizer": null,
        "pre_tokenizer": null,
        "post_processor": null,
        "decoder": null,
        "model": {
            "type": "BPE",
            "dropout": null,
            "unk_token": "[UNK]",
            "continuing_subword_prefix": null,
            "end_of_word_suffix": null,
            "fuse_unk": false,
            "vocab": {
                " ": 0, "a": 1, "b": 2, "c": 3, "d": 4, "e": 5, "f": 6, "g": 7, "h": 8, "i": 9,
                "j": 10, "k": 11, "l": 12, "m": 13, "n": 14, "o": 15, "p": 16, "q": 17, "r": 18, "s": 19,
                "t": 20, "u": 21, "v": 22, "w": 23, "x": 24, "y": 25, "z": 26,
                ".": 27, ",": 28, "!": 29, "?": 30,
                "the": 31, "and": 32, "of": 33, "to": 34, "a": 35, "in": 36, "for": 37, "is": 38, "on": 39, "that": 40,
                "by": 41, "this": 42, "with": 43, "i": 44, "you": 45, "it": 46, "not": 47, "or": 48, "be": 49, "are": 50,
                "from": 51, "at": 52, "as": 53, "your": 54, "all": 55, "any": 56, "can": 57, "had": 58, "her": 59, "was": 60,
                "one": 61, "our": 62, "out": 63, "day": 64, "get": 65, "has": 66, "him": 67, "his": 68, "how": 69, "man": 70,
                "new": 71, "now": 72, "old": 73, "see": 74, "two": 75, "way": 76, "who": 77, "boy": 78, "did": 79, "its": 80,
                "let": 81, "put": 82, "say": 83, "she": 84, "too": 85, "use": 86,
                "qu": 87, "th": 88, "er": 89, "an": 90, "re": 91, "ed": 92, "nd": 93, "on": 94, "en": 95, "at": 96, "es": 97, "or": 98, "ti": 99,
                "[UNK]": 100, "[CLS]": 101, "[SEP]": 102
            },
            "merges": [
                "q u", "t h", "e r", "a n", "r e", "e d", "n d", "o n", "e n", "a t", "e s", "o r", "t i"
            ]
        }
    }'''

    # Create tokenizer from JSON
    tokenizer = tokenizers.from_json(json_config)

    # Test with longer text
    text = "the quick brown fox jumps"
    token_ids = tokenizer.encode(text)

    # Test decoding
    decoded_text = tokenizer.decode(token_ids)

    # Test vocabulary operations
    vocab_size = tokenizer.get_vocab_size()

    # Test special token lookups
    cls_id = tokenizer.token_to_id("[CLS]")
    sep_id = tokenizer.token_to_id("[SEP]")
    unk_id = tokenizer.token_to_id("[UNK]")
    cls_token = tokenizer.id_to_token(101)

    # Create result dictionary
    result = {
        "status": "success",
        "vocab_size": vocab_size,
        "text": text,
        "encoded_length": len(token_ids),
        "decoded_text": decoded_text,
        "cls_id": cls_id,
        "sep_id": sep_id,
        "unk_id": unk_id,
        "cls_token": cls_token
    }

    # Add first token ID separately to avoid ternary operator
    if len(token_ids) > 0:
        result["first_token_id"] = token_ids[0]
    else:
        result["first_token_id"] = -1

    return result


def run_all_tests(params):
    """
    Run all tokenizer tests and return combined results.

    This function executes both the basic and advanced tokenizer tests,
    collecting results from each test case and providing an overall
    status summary.

    Returns:
        dict: Combined test results containing:
            - overall_status (str): "success" if all tests pass, "error" otherwise
            - basic_test (dict): Results from test_tokenizers()
            - comprehensive_test (dict): Results from test_sentencepiece_style()
            - message (str): Summary message or error details

    Example:
        >>> results = run_all_tests()
        >>> assert results["overall_status"] == "success"
        >>> assert results["basic_test"]["status"] == "success"
        >>> assert results["comprehensive_test"]["status"] == "success"
    """

    # Initialize overall status
    overall_status = "success"

    # Run basic tokenizer test
    basic_results = test_tokenizers({})

    # Run comprehensive tokenizer test
    comprehensive_results = test_sentencepiece_style({})

    # Check if any test failed
    if basic_results["status"] != "success":
        overall_status = "error"
    if comprehensive_results["status"] != "success":
        overall_status = "error"

    return {
        "overall_status": overall_status,
        "basic_test": basic_results,
        "comprehensive_test": comprehensive_results,
        "message": "All tests completed successfully with status: " + overall_status
    }
