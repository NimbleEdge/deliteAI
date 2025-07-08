# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import ne_re as re
from delitepy import nimblenet as nm


def main(input):
    # Regex split functions
    split1 = re.split(r'\W+', 'Words, words, words.') # ['Words', 'words', 'words']
    split2 = re.split(r'(\W+)', 'Words, words, words.', True) # ['Words', ', ', 'words', ', ', 'words', '.', '']
    split3 = re.split(r'(\W+)', 'Words, words, words.', False)
    split4 = re.split('[a-f]+', '0a3b9') # ['0', '3', '9']

    # Regex match functions
    match1 = re.match(r"((\w+) (\w+))", "Isaac Newton, physicist")
    match1_group0 = match1.group()
    match1_group1 = match1.group(0)
    match1_group2 = match1.group(3)
    match1_group3 = match1.group(1, 2)
    match1_groups0 = match1.groups()
    match1_start0 = match1.start()
    match1_start1 = match1.start(0)
    match1_start2 = match1.start(3)
    match1_end0 = match1.end()
    match1_end1 = match1.end(0)
    match1_span0 = match1.span(3)

    match2 = re.match(r"(..)+", "a1b2c3")
    match2_group0 = match2.group(1)
    match2_groups0 = match2.groups()
    match2_start0 = match2.start(1)
    match2_span0 = match2.span(1)

    match3 = re.match(r"(\d+)\.(\d+)", "24.1632")
    match3_groups0 = match3.groups()

    match4 = re.match(r"(\d+)\.?(\d+)?", "24")
    match4_groups0 = match4.groups()
    match4_groups1 = match4.groups("1")
    match4_start0 = match4.start()
    match4_start1 = match4.start(2)
    match4_span0 = match4.span()
    match4_end0 = match4.end(2)

    match5 = re.match(".*c", "abcdef")
    match5_group = match5.group()

    match6 = re.match("c", "abcd")
    if not match6:
        print("Match not found for match6")

    # Regex fullmatch functions
    fullmatch1 = re.fullmatch("p.*n", "python")
    fullmatch1_group0 = fullmatch1.group()
    fullmatch1_groups0 = fullmatch1.groups()
    fullmatch1_start = fullmatch1.start()
    fullmatch1_span = fullmatch1.span()

    # Regex search functions
    search1 = re.search("remove_this", "tony@tiremove_thisger.net")
    search1_group0 = search1.group()
    search1_groups0 = search1.groups()

    search2 = re.search("^c", "abcdef")
    # if search2 == None: # This is not supported
    if not search2:
        print("No match found for search2")

    search3 = re.search("^a", "abcdef")  # Match
    search3_group0 = search3.group()
    search3_groups0 = search3.groups()
    search3_groups1 = search3.groups("1")

    # Regex sub functions
    # Below does not work because back reference of groups not supported in std::regex_replace
    # result = re.sub(r'def\s+([a-zA-Z_][a-zA-Z_0-9]*)\s*\(\s*\):',
    #    r'static PyObject*\npy_\1(void)\n{',
    #    'def myfunc():')
    # print(result)

    input = 'Baked Beans And Spam'
    sub0 = re.sub(r'\sAnd\s', ' & ', input)
    sub1 = re.sub(r'a|e|i|o|u', '*', 'Quick brown fox', 1)

    # Regex subn functions
    # re.subn(r'(?<=I have )\d+', '100', 'I have 50 dollars and I have 20 coins') # Does not work
    subn0 = re.subn(r'\sAnd\s', ' & ', 'Baked Beans And Spam')
    subn1 = re.subn(r'a|e|i|o|u', '*', 'Quick brown fox', 2)
    subn2 = re.subn(r'\b\w{3}\b', '***', 'A cat on a hot tin roof')
    subn3 = re.subn(r'<.*?>', '', '<html><body>Text</body></html>')
    subn4 = re.subn(r'\w+', 'word', 'Testing 123!')

    # Regex findall functions
    findall0 = re.findall(r'\bf[a-z]*', 'which foot or hand fell fastest') # No group, all match.size() was 1 so a simple list of strings is returned
    findall1 = re.findall(r'(\w+)=(\d+)', 'set width=20 and height=10') # Two matching groups, need to return two tuples
    findall2 = re.findall(r'(\w+)=', 'set width=20 and height=10') # One matching group, just return a list of strings
    findall3 = re.findall(r'(\w+)\s(\w+)\s\w+', "set width height") # Two matching groups but there are matches ahead, here also just return a tuple of strings
    findall4 = re.findall(r'\w+\s(\w+)\s(\w+)', "set width height") # First some other match then matching group but got matching groups, return a tuple
    findall5 =  re.findall(r'\d+', 'set width')   # No match found
    text = "He was carefully disguised but captured quickly by police."
    findall6 = re.findall(r"\w+ly\b", text)


    # Regex finditer functions
    # Returns a list of match objects, if no match then return Empty List
    text = "He was carefully disguised but captured quickly by police."
    finditer0 = ""
    for m in re.finditer(r"\w+ly\b", text):
        finditer0 = finditer0 + m.group(0)

    finditer1 = ""
    for m in re.finditer(r'\d+', 'fdsf sdf'):
        finditer1 = finditer1 + m.group(0)

    return {
        "split1": split1,
        "split2": split2,
        "split3": split3,
        "split4": split4,
        "match1_group0": match1_group0,
        "match1_group1": match1_group1,
        "match1_group2": match1_group2,
        "match1_group3": nm.tensor([match1_group3[0], match1_group3[1]], "string"), # Tuple to tensor not supported, so first convert to list
        "match1_groups0": nm.tensor([match1_groups0[0], match1_groups0[1], match1_groups0[2]], "string"),
        "match1_start0": match1_start0,
        "match1_start1": match1_start1,
        "match1_start2": match1_start2,
        "match1_end0": match1_end0,
        "match1_end1": match1_end1,
        "match1_span0": nm.tensor([match1_span0[0], match1_span0[1]], "int32"),
        "match2_group0": match2_group0,
        "match2_groups0": nm.tensor([match2_groups0[0]], "string"),
        "match2_start0": match2_start0,
        "match2_span0": nm.tensor([match2_span0[0], match2_span0[1]], "int32"),
        "match3_groups0": nm.tensor([match3_groups0[0], match3_groups0[1]], "string"),
        "match4_groups0_size": len(match4_groups0),
        "match4_groups0": nm.tensor([match4_groups0[0]], "string"),
        "match4_groups1_size": len(match4_groups1),
        "match4_groups1": nm.tensor([match4_groups1[0], match4_groups1[1]], "string"),
        "match4_start0": match4_start0,
        "match4_start1": match4_start1,
        "match4_span0": nm.tensor([match4_span0[0], match4_span0[1]], "int32"),
        "match4_end0": match4_end0,
        "match5_group": match5_group,
        "fullmatch1_group0": fullmatch1_group0,
        "fullmatch1_groups0_size": len(fullmatch1_groups0),
        "fullmatch1_start": fullmatch1_start,
        "fullmatch1_span": nm.tensor([fullmatch1_span[0], fullmatch1_span[1]], "int32"),
        "search1_group0": search1_group0,
        "search1_groups0_size": len(search1_groups0),
        "search3_group0": search3_group0,
        "search3_groups0_size": len(search3_groups0),
        "search3_groups1_size": len(search3_groups1),
        "sub0": sub0,
        "sub1": sub1,
        "subn0_resultstring": subn0[0],
        "subn0_replacements": subn0[1],
        "subn1_resultstring": subn1[0],
        "subn1_replacements": subn1[1],
        "subn2_resultstring": subn2[0],
        "subn2_replacements": subn2[1],
        "subn3_resultstring": subn3[0],
        "subn3_replacements": subn3[1],
        "subn4_resultstring": subn4[0],
        "subn4_replacements": subn4[1],
        "findall0_size": len(findall0),
        "findall0_strings": nm.tensor(findall0, "string"),
        "findall1_size_products": len(findall1) * len(findall1[0]) * len(findall1[1]),
        "findall1_strings": nm.tensor([findall1[0][0], findall1[0][1], findall1[1][0], findall1[1][1]], "string"),
        "findall2_size": len(findall2),
        "findall2_strings": nm.tensor(findall2, "string"),
        "findall3_size_products": len(findall3) * len(findall3[0]),
        "findall3_strings": nm.tensor([findall3[0][1], findall3[0][0]], "string"),
        "findall4_size_products": len(findall4) * len(findall4[0]),
        "findall4_strings": nm.tensor([findall4[0][0], findall4[0][1]], "string"),
        "findall5_size": len(findall5),
        "findall6_size": len(findall6),
        "findall6_strings": nm.tensor(findall6, "string"),
        "finditer0_string": finditer0,
        "finditer1_string": finditer1
    }
