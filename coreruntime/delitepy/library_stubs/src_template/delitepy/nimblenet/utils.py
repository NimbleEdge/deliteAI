# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy.nimblenet.tensor import Tensor

def pow(base:int|float, exp:int|float) -> float:
    """
    Returns base raised to the power exp. The arguments must have numeric type.

    Parameters
    ----------
    base : int|float
    exp : int|float

    Returns
    ----------
    result : float
        Result of base**exp.
    """
    pass

def exp(x:int|float) -> float:
    """
    Returns e raised to the power x, where e=2.718281... is the base of natural logarithms.

    Parameters
    ----------
    x : int|float

    Returns
    ----------
    result : float
        Result of e**x
    """
    pass

def time() -> int:
    """
    Returns the current time elapsed since epoch in seconds.

    Returns
    ----------
    result : int
        Current time elapsed since epoch in seconds.
    """
    pass

def is_integer(s : object) -> bool:
    """
    Returns if the object is an integer

    Returns
    ----------
    result : bool
        true if object passed is an integer, false otherwise
    """

def is_float(s : object) -> bool:
    """
    Returns if the object is an float

    Returns
    ----------
    result : bool
        true if object passed is an float, false otherwise
    """

def is_string(s : object) -> bool:
    """
    Returns if the object is an string

    Returns
    ----------
    result : bool
        true if object passed is an string, false otherwise
    """

def min(tensor : Tensor) -> int|float:
    """
    Returns the minimum element in the tensor

    Returns
    ----------
    result : int|float
        Minimum element in the tensor
    """

def max(tensor : Tensor) -> int|float:
    """
    Returns the maximum element in the tensor

    Returns
    ----------
    result : int|float
        Maximum element in the tensor
    """

def sum(tensor: Tensor) -> int|float:
    """
    Returns the sum of all elements of the tensor

    Returns
    ----------
    result : int|float
        Sum of all elements of the tensor
    """

def mean(tensor: Tensor) -> int|float:
    """
    Returns the mean of all elements of the tensor

    Returns
    ----------
    result : int|float
        Mean of all elements of the tensor
    """

def parse_json(s : str) -> dict:
    """
    Returns the string parsed as JSON

    Returns
    ----------
    result : dict
        JSON object parsed from the string
    """
