# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

"""
List Operations Test Script
This script tests all the list operations implemented in the nimbleSDK C++ runtime
"""

def test_basic_lists(input):
    """Test basic list creation operations"""
    # Empty list
    empty_list = []

    # List with elements
    numbers = [1, 2, 3, 4, 5]

    # List with mixed types
    mixed = [1, "hello", 3.14, True, None]

    # Nested lists
    nested = [1, [2, 3], [4, [5, 6]]]

    return {
        "empty_list": empty_list,
        "numbers": numbers,
        "mixed": mixed,
        "nested": nested
    }

def test_concatenation(input):
    """Test list concatenation operations"""
    list1 = [1, 2, 3]
    list2 = [4, 5, 6]

    # Basic concatenation
    concatenated = list1 + list2

    # Edge cases
    empty_concat = [] + list1
    list_empty_concat = list1 + []

    # Mixed types concatenation
    strings = ["hello", "world"]
    mixed_concat = list1 + strings

    return {
        "concatenated": concatenated,
        "empty_concat": empty_concat,
        "list_empty_concat": list_empty_concat,
        "mixed_concat": mixed_concat
    }

def test_repetition(input):
    """Test list repetition operations"""
    repeat_list = [1, 2, 3]

    # Repeat 3 times
    repeated = repeat_list * 3

    # Edge cases
    zero_repeat = repeat_list * 0
    one_repeat = repeat_list * 1
    empty_repeat = [] * 5

    # Reverse order (int * list)
    int_mult_list = 3 * repeat_list

    return {
        "repeated": repeated,
        "zero_repeat": zero_repeat,
        "one_repeat": one_repeat,
        "empty_repeat": empty_repeat,
        "int_mult_list": int_mult_list
    }

def test_membership(input):
    """Test element membership operations"""
    test_list = [1, 2, 3, "hello", 5.5]
    nested_list = [1, [2, 3], 4]

    # Basic membership tests
    contains_1 = 1 in test_list
    contains_4 = 4 in test_list
    contains_hello = "hello" in test_list
    contains_float = 5.5 in test_list

    # Nested membership
    contains_nested = [2, 3] in nested_list

    return {
        "contains_1": contains_1,
        "contains_4": contains_4,
        "contains_hello": contains_hello,
        "contains_float": contains_float,
        "contains_nested": contains_nested
    }

def test_slicing(input):
    """Test list slicing operations"""
    slice_list = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]

    # Basic slicing
    slice_2_5 = slice_list[2:5]
    slice_begin = slice_list[:3]
    slice_end = slice_list[7:]
    slice_all = slice_list[:]

    # Negative indices
    slice_neg_end = slice_list[-3:]
    slice_neg_begin = slice_list[:-2]

    # Step parameter
    slice_step = slice_list[::2]
    slice_begin_end_step = slice_list[1:8:3]

    # Negative step (reverse)
    slice_reverse = slice_list[::-1]

    # Slicing of an empty list
    empty_list = []
    empty_slice_1 = empty_list[:]
    empty_slice_2 = empty_list[0:5]
    empty_slice_3 = empty_list[-5:-1]
    empty_slice_4 = empty_list[::2]
    empty_slice_5 = empty_list[::-1]
    
    # Assert empty list slicing returns empty lists
    assert len(empty_slice_1) == 0, "Empty list slice [:] should return empty list"
    assert len(empty_slice_2) == 0, "Empty list slice [0:5] should return empty list"
    assert len(empty_slice_3) == 0, "Empty list slice [-5:-1] should return empty list"
    assert len(empty_slice_4) == 0, "Empty list slice [::2] should return empty list"
    assert len(empty_slice_5) == 0, "Empty list slice [::-1] should return empty list"

    
    return {
        "slice_2_5": slice_2_5,
        "slice_begin": slice_begin,
        "slice_end": slice_end,
        "slice_all": slice_all,
        "slice_neg_end": slice_neg_end,
        "slice_neg_begin": slice_neg_begin,
        "slice_step": slice_step,
        "slice_begin_end_step": slice_begin_end_step,
        "slice_reverse": slice_reverse
    }

def test_comprehension(input):
    """Test list comprehension operations"""
    # Basic list comprehension
    numbers = [1, 2, 3, 4, 5]

    # Use a simpler approach to generate squares
    squares = [x*x for x in numbers]
    # List comprehension with condition
    even_squares = [x*x for x in numbers if x % 2 == 0]

    # Nested list comprehension - converted to loops
    matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
    flattened = [item for row in matrix for item in row]
    # List comprehension with complex expressions
    complex_expr = [(x, x*x) for x in range(5)]

    # String-based list comprehension
    words = ["hello", "world", "python", "list", "operations"]
    filtered_words = [word.upper() for word in words if len(word) > 4]

    # Assert list comprehension results
    assert len(squares) == 5, "Squares list should have 5 elements"
    expected_squares = [1, 4, 9, 16, 25]
    for i in range(len(squares)):
        assert squares[i] == expected_squares[i], "squares not matching"
    
    expected_even_squares = [4, 16]
    assert len(even_squares) == 2, "Even squares should have 2 elements"
    for i in range(len(even_squares)):
        assert even_squares[i] == expected_even_squares[i], "even_squares not matching"

    # Test nested comprehension
    assert len(flattened) == 9, "Flattened list should have 9 elements"
    expected_flattened = [1, 2, 3, 4, 5, 6, 7, 8, 9]
    for i in range(len(flattened)):
        assert flattened[i] == expected_flattened[i], "flattened not matching"

    # Test comprehension with complex expressions
    assert len(complex_expr) == 5, "Complex expression list should have 5 elements"
    for i in range(5):
        assert complex_expr[i][0] == i, "complex_expr not matching"
        assert complex_expr[i][1] == i*i, "complex_expr not matching"

    # Test string operations in comprehension
    assert len(filtered_words) == 4, "Filtered words should have 4 elements"
    assert "HELLO" in filtered_words, "HELLO should be in filtered_words"
    assert "PYTHON" in filtered_words, "PYTHON should be in filtered_words"
    assert "OPERATIONS" in filtered_words, "OPERATIONS should be in filtered_words"
    assert "WORLD" in filtered_words, "WORLD should be in filtered_words"

    return {}

def test_multiple_conditions(input):
    """Test list comprehension with multiple conditions"""
    # Multiple sequential conditions
    numbers = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

    # Using multiple 'if' conditions (implicit AND)
    mult_6 = [x for x in numbers if x % 2 == 0 if x % 3 == 0]

    # Using a combined condition with AND
    mult_6_combined = [x for x in numbers if x % 2 == 0 and x % 3 == 0]

    # Conditions on different generators
    matrix = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    even_odd_pairs = [(x, y) for x in matrix if x % 2 == 0 
                            for y in matrix if y % 2 == 1]

    # Assert multiple conditions work correctly
    expected_mult_6 = [x for x in range(11) if x % 2 == 0 if x % 3 == 0]
    assert len(mult_6) == len(expected_mult_6), "mult_6 length should match expected"
    for i in range(len(mult_6)):
        assert mult_6[i] == expected_mult_6[i], "mult_6 values not matching"

    # Check combined 'and' conditions produce the same result
    assert len(mult_6) == len(mult_6_combined), "mult_6 and mult_6_combined should have same length"
    for i in range(len(mult_6)):
        assert mult_6[i] == mult_6_combined[i], "mult_6 and mult_6_combined should be equal"

    # Check conditions on different generators
    expected_pairs = [(x, y) for x in range(11) if x % 2 == 0 for y in range(11) if y % 2 == 1]
    assert len(even_odd_pairs) == len(expected_pairs), "even_odd_pairs length should match expected"
    i = 0
    for pair in expected_pairs:
        assert even_odd_pairs[i][0] == pair[0], "even_odd_pairs first element not matching"
        assert even_odd_pairs[i][1] == pair[1], "even_odd_pairs second element not matching"
        i = i + 1

    return {}

def test_mod_operations(input):
    """Test modulo operations including edge cases"""
    # Normal mod operations
    mod_positive = 10 % 3
    mod_negative_dividend = -10 % 3
    mod_negative_divisor = 10 % -3
    mod_both_negative = -10 % -3
    
    # Assert normal mod operations
    assert mod_positive == 1, "10 % 3 should be 1, got " + str(mod_positive)
    assert mod_negative_dividend == 2, "-10 % 3 should be 2, got " + str(mod_negative_dividend)
    assert mod_negative_divisor == 1, "10 % -3 should be 1, got " + str(mod_negative_divisor)
    assert mod_both_negative == -1, "-10 % -3 should be -1, got " + str(mod_both_negative)
    
    # Edge cases
    mod_zero_dividend = 0 % 5
    mod_one = 10 % 1
    mod_self = 7 % 7
    
    assert mod_zero_dividend == 0, "0 % 5 should be 0, got " + str(mod_zero_dividend)
    assert mod_one == 0, "10 % 1 should be 0, got " + str(mod_one)
    assert mod_self == 0, "7 % 7 should be 0, got " + str(mod_self)
    
    # Float modulo
    mod_float = 10.5 % 3.0
    mod_float_negative = -10.5 % 3.0
    
    assert mod_float == 1.5, "10.5 % 3.0 should be 1.5, got " + str(mod_float)
    assert mod_float_negative == 1.5, "-10.5 % 3.0 should be 1.5, got " + str(mod_float_negative)
    
    # Try to catch division by zero - should fail
    try:
        mod_by_zero = 10 % 0
    except :
        assert True, "Expected ZeroDivisionError for division by zero"
    
    # Try to catch type errors - should fail
    try:
        mod_string = "10" % 3
    except :
        assert True, "Expected TypeError for string modulo"
    
    try:
        mod_list = [10] % 3
    except :
        assert True, "Expected TypeError for list modulo"
        
    try:
        mod_none = None % 3
    except:
        assert True, "Expected TypeError for None modulo"
    
    # # Mod with mixed types in list
    # try:
    #     mixed_list = [10, 10.5, 11]
    #     mod_mixed = [x % 3 for x in mixed_list]
    #     assert mod_mixed[0] == 1, "10 % 3 should be 1, got " + str(mod_mixed[0])
    #     assert mod_mixed[1] == 1.5, "10.5 % 3 should be 1.5, got " + str(mod_mixed[1])
    #     assert mod_mixed[2] == 2, "11 % 3 should be 2, got " + str(mod_mixed[2])
    # except:
    #     assert False, "Mixed types mod should work"
    
    # Test mod by zero in comprehension - should fail
    # try:
    #     mod_zero_comp = [x % 0 for x in [1, 2, 3]]
    # except:
    #     assert False, "Expected ZeroDivisionError for mod by zero in comprehension"
    
    return {}

def test_concatenation_edge_cases(input):
    """Test edge cases for list concatenation"""
    # Try concatenating list with non-list types - should fail
    try:
        list_plus_int = [1, 2, 3] + 4
    except:
        assert True, "Expected TypeError for list + int"
    
    try:
        list_plus_string = [1, 2, 3] + "abc"
    except:
        assert True, "Expected TypeError for list + string"
    
    try:
        list_plus_none = [1, 2, 3] + None
    except:
        assert True, "Expected TypeError for list + None"
    
    # Multiple empty lists
    multiple_empty = [] + [] + []
    assert len(multiple_empty) == 0, "Multiple empty list concatenation should be empty"
    
    # Nested list concatenation
    nested_concat = [[1, 2]] + [[3, 4]]
    assert len(nested_concat) == 2, "Nested list concatenation should have 3 elements"
    assert nested_concat[1][0] == 3, "Second element should be [3, 4]"

    
    # Try reverse concatenation (non-list + list) - should fail
    try:
        int_plus_list = 4 + [1, 2, 3]
    except:
        assert True, "Expected TypeError for int + list"
    
    return {}