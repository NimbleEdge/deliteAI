# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy.ne_re.match import Match

def split(pattern: str, string: str, return_matched_groups: bool = False) -> list[str]:
    """
    Split string by the occurrences of pattern. If capturing parentheses are used in pattern, then the text of all groups in the pattern are also returned as part of the resulting list if return_matched_groups is set to True.

    Examples
    --------
    >>> ne_re.split(r'\W+', 'Words, words, words.') # ['Words', 'words', 'words']
    >>> ne_re.split(r'(\W+)', 'Words, words, words.', True) # ['Words', ', ', 'words', ', ', 'words', '.']
    >>> ne_re.split(r'(\W+)', 'Words, words, words.', False) # ['Words', 'words', 'words']
    >>> ne_re.split('[a-f]+', '0a3b9') # ['0', '3', '9']

    Parameters
    ----------
    pattern: str
        regex pattern to be used for searching
    string: str
        input string on which to search
    return_matched_groups: bool
        Whether matched groups should also be returned in the output

    Returns
    ----------
    splits:
        List of strings after splitting the original string
    """
    pass

def match(pattern: str, string: str) -> Match | None:
    """
    If zero or more characters at the beginning of string match the regular expression pattern, return a corresponding Match.
    Return None if the string does not match the pattern.

    Examples
    --------
    >>> m = ne_re.match(r"(\w+) (\w+)", "Isaac Newton, physicist")
    >>> m.group(0)       # 'Isaac Newton'
    >>> m.group(1)       # 'Isaac'
    >>> m.group(2)       # 'Newton'
    >>> m.group(1, 2)    # ('Isaac', 'Newton')

    Parameters
    ----------
    pattern: str
        regex pattern to be used for searching
    string: str
        input string on which to search

    Returns
    ----------
    Match if pattern found else None.
    """
    pass

def fullmatch(pattern: str, string: str) -> Match | None:
    """
    If the whole string matches the regular expression pattern, return a corresponding Match.
    Return None if the string does not match the pattern.

    Examples
    --------
    >>> ne_re.fullmatch("p.*n", "python") # <ne_re.Match object; span=(0, 6), match='python'>
    >>> ne_re.fullmatch("r.*n", "python") # None

    Parameters
    ----------
    pattern: str
        regex pattern to be used for searching
    string: str
        input string on which to search

    Returns
    ----------
    Match if pattern found else None.
    """
    pass

def search(pattern: str, string: str) -> Match | None:
    """
    Scan through string looking for the first location where the regular expression pattern produces a match, and return a corresponding Match.
    Return None if no position in the string matches the pattern.

    Parameters
    ----------
    pattern: str
        regex pattern to be used for searching
    string: str
        input string on which to search

    Returns
    ----------
    Match if pattern found else None.
    """
    pass

def sub(pattern: str, replacement: str, string: str, count: int = 0) -> str:
    """
    Return the string obtained by replacing the leftmost non-overlapping occurrences of pattern in string by the replacement. If the pattern isn't found, string is returned unchanged.

    Examples
    --------
    >>> ne_re.sub(r'\sAnd\s', ' & ', 'Baked Beans And Spam') # 'Baked Beans & Spam'

    Parameters
    ----------
    pattern: str
        regex pattern to be used for searching
    replacement: str
        replacement string to be used when a pattern is found
    string: str
        input string on which to search
    count: int
        maximum number of replacements to be done

    Returns
    ----------
    Modified string after substituting replacement in string for pattern

    """
    pass

def subn(pattern: str, replacement: str, string: str, count: int = 0) -> tuple[str, int]:
    """
    Perform the same operation as sub(), but return a tuple (new_string, number_of_subs_made).

    Parameters
    ----------
    pattern: str
        regex pattern to be used for searching
    replacement: str
        replacement string to be used when a pattern is found
    string: str
        input string on which to search
    count: int
        maximum number of replacements to be done

    Returns
    ----------
    Tuple with modified string after substituting replacement in string for pattern and count of substitutions applied.

    """
    pass

def findall(pattern: str, string: str) -> list[str] | list[tuple[str]]:
    """
    Return all non-overlapping matches of pattern in string, as a list of strings or tuples.
    The string is scanned left-to-right, and matches are returned in the order found. Empty matches are included in the result.

    The result depends on the number of capturing groups in the pattern. If there are no groups, return a list of strings matching the whole pattern.
    If there is exactly one group, return a list of strings matching that group. If multiple groups are present, return a list of tuples of strings matching the groups. Non-capturing groups do not affect the form of the result.

    Examples
    --------
    >>> ne_re.findall(r'\bf[a-z]*', 'which foot or hand fell fastest') # ['foot', 'fell', 'fastest']
    >>> ne_re.findall(r'(\w+)=(\d+)', 'set width=20 and height=10') # [('width', '20'), ('height', '10')]

    Parameters
    ----------
    pattern: str
        regex pattern to be used for searching
    string: str
        input string on which to search

    Returns
    ----------
    List of strings or a list of tuple of strings

    """
    pass

def finditer(pattern: str, string: str) -> list[Match]:
    """
    Return a list of Match objects over all non-overlapping matches for the regex pattern in string.
    The string is scanned left-to-right, and matches are returned in the order found. Empty matches are included in the result.

    Parameters
    ----------
    pattern: str
        regex pattern to be used for searching
    string: str
        input string on which to search

    Returns
    ----------
    List of match objects
    """
    pass
