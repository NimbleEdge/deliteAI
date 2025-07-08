# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

def test_ascii_string_slicing(input):
    s = input["s"]
    # Test basic ASCII string slicing
    return {
        "s[0:5:1]": s[0:5:1],
        "s[7:12:1]": s[7:12:1],
        "s[0::2]": s[0::2],
        "s[::-1]": s[::-1],
        "s[5:7:1]": s[5:7:1],
        "s[-6:-1:1]": s[-6:-1:1],
    }


def test_unicode_string_slicing(input):
    s = input["s"]
    # Test Unicode string slicing
    return {
        "s[0:5:1]": s[0:5:1],
        "s[7:9:1]": s[7:9:1],
        "s[-3::1]": s[-3::1],
        "s[0::2]": s[0::2],
        "s[::-1]": s[::-1],
    }

def test_edge_cases(input):
    s = input["s"]
    # Test edge cases
    return {
        "s[0:0:1]": s[0:0:1],
        "s[100:200:1]": s[100:200:1],
        "s[-100:-50:1]": s[-100:-50:1],
        "s[2:2:1]": s[2:2:1],
        "s[::1]": s[::1],
    }

def test_mixed_unicode_slicing(input):
    s = input["s"]
    # Test mixed ASCII and Unicode characters
    return {
        "s[6:8:1]": s[6:8:1],
        "s[9:11:1]": s[9:11:1],
        "s[11:13:1]": s[11:13:1],
        "s[0::3]": s[0::3],
        "s[::-2]": s[::-2],
    }
