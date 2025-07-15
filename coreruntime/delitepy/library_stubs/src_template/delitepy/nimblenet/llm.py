# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

"""Type stubs for delitepy.nimblenet.llm module."""

from typing import Dict, Any, Union, Optional


class JSONArrayIterator:
    """Iterator for JSON array streams."""

    def next(self) -> Optional['JSONValueStream']:
        """Get the next element from the array iterator.

        Returns
        -------
        Optional[JSONValueStream]
            The next JSONValueStream element, or None if no more elements.
        """
        pass

    def next_available(self) -> bool:
        """Check if the next element is available.

        Returns
        -------
        bool
            True if the next element is available, False otherwise.
        """
        pass

    def next_blocking(self) -> Optional['JSONValueStream']:
        """Get the next element from the array iterator, blocking until available.

        Returns
        -------
        Optional[JSONValueStream]
            The next JSONValueStream element, or None if no more elements.
        """
        pass


class JSONValueStream:
    """JSON value stream for parsing and accessing JSON data from character streams."""

    def finished(self) -> bool:
        """Check if the JSON stream has finished parsing.

        Returns
        -------
        bool
            True if the stream has finished, False otherwise.
        """
        pass

    def iterator(self) -> JSONArrayIterator:
        """Get an iterator for array elements.

        Returns
        -------
        JSONArrayIterator
            A JSONArrayIterator for iterating over array elements.

        Raises
        ------
        UnsupportedError
            If the stream is not an array type.
        """
        pass

    def get_blocking(self, key: str) -> 'JSONValueStream':
        """Get a JSON value stream for a specific key, blocking until available.

        Parameters
        ----------
        key : str
            The key to look up in the JSON object.

        Returns
        -------
        JSONValueStream
            A JSONValueStream for the specified key.

        Raises
        ------
        UnsupportedError
            If the stream is not an object type.
        KeyError
            If the key is not found in the JSON object.
        """
        pass

    def get_blocking_str(self, key: str) -> str:
        """Get a string value for a specific key, blocking until available.

        Parameters
        ----------
        key : str
            The key to look up in the JSON object.

        Returns
        -------
        str
            The string value for the specified key.

        Raises
        ------
        UnsupportedError
            If the stream is not an object type.
        KeyError
            If the key is not found in the JSON object.
        """
        pass

    def wait_for_completion(self) -> None:
        """Wait for the JSON stream to complete parsing."""
        pass

    def __str__(self) -> str:
        """Convert the JSON value to a string representation.

        Returns
        -------
        str
            String representation of the JSON value.
        """
        pass


class CharStream:
    """Character stream for streaming LLM responses."""

    def finished(self) -> bool:
        """Check if the stream has finished generating content.

        Returns
        -------
        bool
            True if the stream has finished, False otherwise.
        """
        pass

    def next(self) -> str:
        """Get the next string chunk from the stream.

        Returns
        -------
        str
            The next string chunk from the LLM response.
        """
        pass

    def skip_text_and_get_json_stream(self) -> JSONValueStream:
        """Skip text content and get a JSON stream for parsing JSON data.

        This method skips over any text content until it finds a valid JSON character,
        then returns a JSONValueStream for parsing the JSON object.

        Returns
        -------
        JSONValueStream
            A JSONValueStream for parsing the JSON content.

        Examples
        --------
        >>> char_stream = llm.prompt("Generate JSON: {\"key\": \"value\"}")
        >>> json_stream = char_stream.skip_text_and_get_json_stream()
        >>> value = json_stream.get_blocking_str("key")
        """
        pass


class LLM:
    """Large Language Model interface for text generation and conversation management."""

    def __init__(self, config: Dict[str, Any]) -> None:
        """Initialize an LLM instance with the given configuration.

        Parameters
        ----------
        config : Dict[str, Any]
            Configuration dictionary containing model parameters.
            Must include 'name' field specifying the model name.
            May include 'provider' and 'metadata' fields.

        Examples
        --------
        >>> llm = LLM({"name": "llama-3"})
        >>> llm = LLM({"name": "gemini:nano:on-device", "provider": "os"})
        >>> llmMetadata = {
        ...     "endOfTurnToken": "\\<\\|eot_id\\|\\>",
        ...     "maxTokensToGenerate": 2000,
        ...     "tokenizerFileName": "tokenizer.bin",
        ...     "temperature": 0.8
        ... }
        >>> llm = LLM({"name": "llama-3", "metadata": llmMetadata})
        """

    def prompt(self, prompt: str) -> CharStream:
        """Generate a response to the given prompt.

        Parameters
        ----------
        prompt : str
            The input prompt string to send to the LLM.

        Returns
        -------
        CharStream
            A CharStream object for streaming the LLM response.

        Examples
        --------
        >>> stream = llm.prompt("What is the capital of France?")
        >>> while not stream.finished():
        ...     chunk = stream.next()
        ...     print(chunk)
        """
        pass

    def cancel(self) -> None:
        """Cancel ongoing text generation.

        Stops the current LLM generation process if one is running.
        This method can be called to interrupt a long-running generation.
        """
        pass

    def add_context(self, context: str) -> None:
        """Add context to the LLM's conversation history e.g. loading a past
        conversation.

        Parameters
        ----------
        context : str
            The context string to add to the conversation history.
            This can include system prompts, user messages, or assistant responses.

        Examples
        --------
        >>> llm.add_context("<|start_header_id|>system<|end_header_id|>You are a helpful assistant.<|eot_id|>")
        """
        pass

    def clear_context(self) -> None:
        """Clear the LLM's conversation history and reset the model context.

        This method removes all previously added context and conversation history,
        effectively resetting the LLM to its initial state. This is useful when
        you want to start a fresh conversation or clear sensitive information
        from the model's memory.

        Examples
        --------
        >>> llm.add_context("Previous conversation context...")
        >>> llm.prompt("Hello, how are you?")
        >>> llm.clear_context()  # Reset to clean state
        >>> llm.prompt("What's the weather?")  # Fresh conversation
        """
        pass

