# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

"""Package delitepy.tokenizers for tokenizer functionality."""

from typing import List, Union
from delitepy.nimblenet.tensor import Tensor

def from_pretrained(model_name_or_path: str) -> str:
    """Load a pre-trained tokenizer from HuggingFace Hub or local file.
    
    Args:
        model_name_or_path: Path to tokenizer.json file or HuggingFace model name
        
    Returns:
        Tokenizer handle (opaque string identifier)
        
    Example:
        >>> tokenizer = tokenizers.from_pretrained("bert-base-uncased")
        >>> tokenizer = tokenizers.from_pretrained("/path/to/tokenizer.json")
    """
    pass

def from_file(file_path: str) -> str:
    """Load a tokenizer from a file path.
    
    Args:
        file_path: Path to tokenizer.json or .model file
        
    Returns:
        Tokenizer handle (opaque string identifier)
        
    Example:
        >>> tokenizer = tokenizers.from_file("tokenizer.json")
        >>> tokenizer = tokenizers.from_file("model.spm")
    """
    pass

def from_json(json_str: str) -> str:
    """Create a tokenizer from a JSON string.
    
    Args:
        json_str: JSON string containing tokenizer configuration
        
    Returns:
        Tokenizer handle (opaque string identifier)
        
    Example:
        >>> json_config = '{"model": {...}, "normalizer": {...}}'
        >>> tokenizer = tokenizers.from_json(json_config)
    """
    pass

def from_sentencepiece(model_path: str) -> str:
    """Load a SentencePiece tokenizer from a .model file.
    
    Args:
        model_path: Path to SentencePiece .model file
        
    Returns:
        Tokenizer handle (opaque string identifier)
        
    Example:
        >>> tokenizer = tokenizers.from_sentencepiece("tokenizer.model")
    """
    pass

def encode(tokenizer: str, text: str) -> Tensor:
    """Encode text into token IDs.
    
    Args:
        tokenizer: Tokenizer handle from from_pretrained/from_file/etc.
        text: Text to encode
        
    Returns:
        Tensor containing token IDs (INT32)
        
    Example:
        >>> tokenizer = tokenizers.from_pretrained("bert-base-uncased")
        >>> token_ids = tokenizers.encode(tokenizer, "Hello world!")
        >>> print(token_ids.shape)  # [num_tokens]
    """
    pass

def decode(tokenizer: str, token_ids: Tensor) -> str:
    """Decode token IDs back to text.
    
    Args:
        tokenizer: Tokenizer handle
        token_ids: Tensor containing token IDs (INT32)
        
    Returns:
        Decoded text string
        
    Example:
        >>> tokenizer = tokenizers.from_pretrained("bert-base-uncased")
        >>> token_ids = tokenizers.encode(tokenizer, "Hello world!")
        >>> text = tokenizers.decode(tokenizer, token_ids)
        >>> print(text)  # "Hello world!"
    """
    pass

def get_vocab_size(tokenizer: str) -> int:
    """Get the vocabulary size of the tokenizer.
    
    Args:
        tokenizer: Tokenizer handle
        
    Returns:
        Size of the vocabulary
        
    Example:
        >>> tokenizer = tokenizers.from_pretrained("bert-base-uncased")
        >>> vocab_size = tokenizers.get_vocab_size(tokenizer)
        >>> print(vocab_size)  # 30522
    """
    pass

def token_to_id(tokenizer: str, token: str) -> int:
    """Convert a token string to its ID.
    
    Args:
        tokenizer: Tokenizer handle
        token: Token string
        
    Returns:
        Token ID, or -1 if token not found
        
    Example:
        >>> tokenizer = tokenizers.from_pretrained("bert-base-uncased")
        >>> token_id = tokenizers.token_to_id(tokenizer, "[CLS]")
        >>> print(token_id)  # 101
    """
    pass

def id_to_token(tokenizer: str, token_id: int) -> str:
    """Convert a token ID to its string representation.
    
    Args:
        tokenizer: Tokenizer handle
        token_id: Token ID
        
    Returns:
        Token string, or empty string if ID not found
        
    Example:
        >>> tokenizer = tokenizers.from_pretrained("bert-base-uncased")
        >>> token = tokenizers.id_to_token(tokenizer, 101)
        >>> print(token)  # "[CLS]"
    """
    pass 