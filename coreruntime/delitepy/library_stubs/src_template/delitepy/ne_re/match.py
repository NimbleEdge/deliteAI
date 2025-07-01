class Match:
    def group(self, *args: int) -> str | tuple[str]:
        r"""
        Returns one or more subgroups of the match. If there is a single argument, the result is a single string; if there are multiple arguments, the result is a tuple with one item per argument.
        Without arguments, group1 defaults to zero (the whole match is returned). If a groupN argument is zero, the corresponding return value is the entire matching string; If a group number is negative or larger than the number of groups defined in the pattern, an exception is raised.
        If a group is contained in a part of the pattern that did not match, the corresponding result is None. If a group is contained in a part of the pattern that matched multiple times, the last match is returned.

        ```python
            m = ne_re.match(r"(\w+) (\w+)", "Isaac Newton, physicist")
            m.group(0)       # 'Isaac Newton'
            m.group(1)       # 'Isaac'
            m.group(2)       # 'Newton'
            m.group(1, 2)    # ('Isaac', 'Newton')
        ```
        Note:
        Named groups are not supported like regex pattern containing (?P<name>...) syntax.

        If a group matches multiple times, only the last match is accessible:
        ```python
        m = ne_re.match(r"(..)+", "a1b2c3")  # Matches 3 times.
        m.group(1)                        # 'c3'
        ```

        Parameters
        ----------
        args : *int
            zero or more indices of groups

        Returns
        ----------
        string or tuple of strings conatining matched groups
        """
        pass

    def groups(self, default: str = None) -> tuple[str|None]:
        r"""
        Return a tuple containing all the subgroups of the match, from 1 up to however many groups are in the pattern.
        The default argument is used for groups that did not participate in the match; it defaults to None.

        ```python
            m = ne_re.match(r"(\d+)\.(\d+)", "24.1632")
            m.groups()                  # ('24', '1632')
            m = ne_re.match(r"(\d+)\.?(\d+)?", "24")
            m.groups()                  # ('24', None)
            m.groups('0')               # ('24', '0')
        ```

        Parameters
        ----------
        default : str
            Default string to return if matched groups is None

        Returns
        ----------
        Tuple of strings or None for matched groups from index 1

        """
        pass

    def start(self, index: int = 0) -> int:
        """
        Return the indices of the start of the substring matched by group; group defaults to zero (meaning the whole matched substring).
        Return -1 if group exists but did not contribute to the match.

        Parameters
        ----------
        index : str
            group index for which starting position in input string is required

        Returns
        ----------
        starting position in input string
        """
        pass

    def end(self, index: int = 0) -> int:
        """
        Return the indices of the end of the substring matched by group; group defaults to zero (meaning the whole matched substring).
        Return -1 if group exists but did not contribute to the match.

        Parameters
        ----------
        index : str
            group index for which ending position in input string is required

        Returns
        ----------
        ending position in input string
        """
        pass

    def span(self, index: int = 0) -> tuple[int, int]:
        """
        Return the tuple of start and end indices of the substring matched by group; group defaults to zero (meaning the whole matched substring).
        Return (-1, -1) if group exists but did not contribute to the match.

        Parameters
        ----------
        index : str
            group index for which starting and ending position in input string is required

        Returns
        ----------
        tuple with starting and ending position
        """
        pass
