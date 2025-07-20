# Minimal Python Constraints for DeliteAI Simulator

This document outlines all the constraints and limitations when writing Python code for the DeliteAI simulator's minimal Python implementation. These constraints were discovered while building a Qwen tokenizer for the simulator environment.

## Table of Contents
1. [Built-in Functions Not Available](#built-in-functions-not-available)
2. [Language Features Not Supported](#language-features-not-supported)
3. [Standard Library Limitations](#standard-library-limitations)
4. [Function Definition Constraints](#function-definition-constraints)
5. [Data Structure Limitations](#data-structure-limitations)
6. [Control Flow Restrictions](#control-flow-restrictions)
7. [String Handling](#string-handling)
8. [Import Restrictions](#import-restrictions)
9. [Best Practices](#best-practices)

## Built-in Functions Not Available

The following built-in functions are NOT available in the minimal Python environment:

- `ord()` - Cannot convert characters to Unicode code points
- `chr()` - Cannot convert Unicode code points to characters
- `isinstance()` - Cannot check object types
- `hasattr()` - Cannot check if object has attribute
- `setattr()` - Cannot dynamically set attributes
- `getattr()` - Limited or not available
- `enumerate()` - Cannot enumerate with index
- `zip()` - Cannot zip iterables together
- `round()` - Cannot round numbers
- `eval()` - Cannot evaluate strings as code
- `exec()` - Cannot execute dynamic code
- `compile()` - Cannot compile code
- `globals()` / `locals()` - Cannot access namespaces
- `vars()` - Cannot get object's __dict__
- `dir()` - Cannot list attributes
- `help()` - No interactive help
- `input()` - No user input
- `open()` - File operations limited or unavailable

## Language Features Not Supported

### 1. Function Definitions
- **NO default parameter values**: Cannot use `def func(param=default)`
- **NO *args or **kwargs**: Cannot use variable arguments
- **NO keyword-only arguments**: Cannot use `def func(*, kwonly)`
- **NO decorators**: Cannot use `@decorator` syntax
- **NO lambda functions**: Cannot use `lambda x: x + 1`

### 2. Operators and Expressions
- **NO ternary operators**: Cannot use `x if condition else y`
- **NO walrus operator**: Cannot use `:=`
- **NO unpacking with `*`**: Cannot use `first, *rest = items`
- **NO `**` for kwargs**: Cannot use `func(**dict)`
- **NO f-strings**: Cannot use `f"Hello {name}"`

### 3. Comparisons
- **NO `is` / `is not`**: Must use `==` / `!=` instead
- Be careful with None comparisons: use `== None` not `is None`

### 4. Comprehensions and Generators
- List comprehensions work but with limitations
- **NO generator expressions**: Cannot use `(x for x in items)`
- **NO dict/set comprehensions**: Limited support

## Standard Library Limitations

The following standard library modules are NOT available:
- `os` - No operating system interface
- `sys` - Limited or no system-specific parameters
- `json` - No JSON parsing/serialization
- `re` - Use `delitepy.ne_re` instead (with limitations)
- `datetime` - No date/time handling
- `time` - No time functions
- `unicodedata` - No Unicode database
- `functools` - No functional programming tools
- `itertools` - No iteration tools
- `collections` - No specialized containers
- `dataclasses` - No dataclass decorator
- `typing` - No type hints
- `pathlib` - No path handling
- `urllib` - No URL handling
- `subprocess` - No subprocess execution

## Function Definition Constraints

### Correct Way:
```python
def my_function(param1, param2):
    """Function with all parameters required"""
    return param1 + param2
```

### Incorrect Ways:
```python
# NO default values
def my_function(param1, param2="default"):  # ❌
    pass

# NO *args
def my_function(*args):  # ❌
    pass

# NO **kwargs
def my_function(**kwargs):  # ❌
    pass

# NO decorators
@decorator  # ❌
def my_function():
    pass
```

## Data Structure Limitations

### Built-in Constructors
- `list()` - NOT available, use `[]`
- `dict()` - NOT available, use `{}`
- `set()` - NOT available
- `tuple()` - Limited availability
- `range()` - Available but use carefully
- `bytes()` / `bytearray()` - NOT available

### Dictionary Methods
- `.get(key, default)` - NOT available, use:
  ```python
  # Instead of: value = dict.get(key, default)
  if key in dict:
      value = dict[key]
  else:
      value = default
  ```

### List Methods
- Most basic methods work: `.append()`, `.extend()`, `.pop()`
- Be careful with advanced methods

## Control Flow Restrictions

### Conditionals
```python
# Correct
if condition:
    do_something()
else:
    do_other()

# Incorrect - NO ternary
value = x if condition else y  # ❌

# Must use:
if condition:
    value = x
else:
    value = y
```

### Loops
```python
# Correct - simple for loop
for item in items:
    process(item)

# Incorrect - NO enumerate
for i, item in enumerate(items):  # ❌
    process(i, item)

# Must use:
i = 0
for item in items:
    process(i, item)
    i = i + 1
```

### Exception Handling
- Basic try/except works
- Avoid complex exception handling
- Don't reuse exception variable names in nested blocks

## String Handling

### String Formatting
```python
# NO f-strings
text = f"Hello {name}"  # ❌

# Use concatenation
text = "Hello " + name  # ✓

# Or format with str()
text = "Value: " + str(number)  # ✓
```

### String Methods
- Basic methods work: `.strip()`, `.split()`, `.join()`
- No `.format()` method
- No `%` formatting

## Import Restrictions

### Local Imports
- Only support: `from module import item`
- NO dot notation: `import module.submodule` ❌
- NO aliasing might be limited: `import module as m` ⚠️

### Example:
```python
# Correct
from delitepy import nimblenet
from delitepy import ne_re

# Incorrect
import delitepy.nimblenet  # ❌
import os  # ❌ (not available)
```

## Best Practices

### 1. Variable Initialization
Always initialize variables before use:
```python
# Good
result = None
for item in items:
    result = process(item)

# Bad - result might be undefined
for item in items:
    result = process(item)  # ❌ if items is empty
```

### 2. Type Checking
Since `isinstance()` is not available:
```python
# Cannot do:
if isinstance(obj, str):  # ❌
    pass

# Try alternative approaches or avoid type checking
```

### 3. Simplify Logic
- Avoid complex expressions
- Break down operations into simple steps
- Use explicit if/else instead of clever shortcuts

### 4. Manual Implementations
Many built-in functions need manual implementation:
```python
# Manual enumerate
i = 0
for item in items:
    # Use i as index
    i = i + 1

# Manual round (to 1 decimal)
value = int(number * 10) / 10.0

# Manual zip (for two lists)
result = []
for i in range(len(list1)):
    if i < len(list2):
        result.append((list1[i], list2[i]))
```

### 5. Error Handling
- Always provide fallbacks
- Initialize variables properly
- Check for None/empty conditions explicitly

## Example: Minimal Tokenizer Structure

```python
# Minimal tokenizer compatible with all constraints
from delitepy import ne_re

class MinimalTokenizer:
    def __init__(self, vocab):
        self.vocab = vocab
        self.reverse_vocab = {}
        for k, v in vocab.items():
            self.reverse_vocab[v] = k
    
    def tokenize(self, text):
        # Simple tokenization with ne_re
        pattern = r"[a-zA-Z]+|[0-9]+|[^\sa-zA-Z0-9]+"
        matches = ne_re.findall(pattern, text)
        if matches == None:
            return []
        return matches
    
    def encode(self, text):
        tokens = self.tokenize(text)
        ids = []
        for token in tokens:
            if token in self.vocab:
                ids.append(self.vocab[token])
            else:
                ids.append(0)  # Unknown token ID
        return ids
```

## Testing in Simulator

When testing your code:
1. Start with the simplest possible implementation
2. Add features incrementally
3. Test each constraint violation separately
4. Keep functions small and focused
5. Avoid deeply nested structures

## Summary

The minimal Python environment is extremely limited compared to standard Python. When in doubt:
- Use the simplest possible approach
- Avoid advanced Python features
- Implement functionality manually
- Test incrementally
- Keep code explicit and straightforward 