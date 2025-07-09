# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

"""Package delitepy.nimblenet."""

from typing import Dict, Any

from delitepy.nimblenet.eventstore import *
from delitepy.nimblenet.tensor import *
from delitepy.nimblenet.utils import *
from delitepy.nimblenet.llm import *

def get_config()->dict:
    """
    Get the config defined during nimbleedge sdk initialization.
    Returns a dict with two keys cohortIds and compatibilityTag.

    Returns
    ----------
    config : dict

    Example
    ----------
    config = nm.get_config()

    tag = config["compatibilityTag"] # Will return the compatibilityTag
    """
    pass

def llm(config: Dict[str, Any]) -> LLM:
    """Create an LLM instance with the given configuration.

    This is a factory function that creates and returns an LLM instance.

    Parameters
    ----------
    config : Dict[str, Any]
        Configuration dictionary containing model parameters.
        Must include 'name' field specifying the model name.
        May include 'provider' and 'metadata' fields.

    Returns
    -------
    LLM
        An LLM instance configured with the specified parameters.

    Examples
    --------
    >>> llm_instance = llm({"name": "llama-3"})
    """
    pass
