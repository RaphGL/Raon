# RAON Specification

```grammar
entry_list = (entry separator)+
entry      = key ("." key)* "=" value
key    = ident | num | string
value  = (string | bool | num | block | array)

ident  = (A-z | "_") (A-z | "_" | 0-9 | "-")*
string = "\"" [\W\d]* "\"" # a string can be any valid unicode
bool   = "true" | "false"
block      = "{" entry_list "}" 
array      = "[" (value separator)* "]"

num         = hex_num | octal_num | decimal_num | binary_num | float_num
hex_num     = "0x" (A-f | "_")+
octal_num   = "0o" (0-7 | "_")+
decimal_num = "-"? (0-9 | "_")+
binary_num  = "0b" (0-1 | "_")+
float_num   = "-"? (0-9 | "_")+ "." (0-9 | "_")+

# end_char here means the ending character for arrays or blocks
# it only is valid when closing an array or a block
# this exists so that arrays or blocks can be inlined without trailing commas
separator = "," | "\n" | end_char
```

Grammar syntax:
- `=`: starts a grammar point declaration.
- `*`: the previous token is repeated zero or more times. 
- `?`: the previous token is repeated 0 or 1 time.
- `+`: the previous token is repeated 1 or more times.
- `""`: anything in quotes is a literal.
- `()`: used for grouping, for example `(num "+" num)*` means that this will happen 0 or more times.
- `|`: used to denote that either the left or the right side will happen, for example `string | bool` means that this will either be
whatever is defined in the string grammar point or in the bool grammar point.
- `0-9` indicates a number range from 0 to 9

### Top-level

A raon file should always start with an entry list. A top-level block or array literal are invalid.

**Rationale**: Raon should easily map to hashtable or when possible directly to a struct (which might be unmarshaled into through reflection in some languages).

### Primitive types
There's only 5 types in Raon:
- int: represents signed integers, it should ideally be the highest indexable size in the system (`intptr_t` in C or `isize` in many other languages)
- float: should ideally represent a float64 number (usually `double` or `f64` in many languages)
- bool: represents a boolean type
- string: any valid language unicode character
- array: an array of items of one of the types
- block: an associative array whose values are of the other types

### Comments
They start with a `#` and stop once a newline is found.

**Rationale:** an inline comment can be used to write multiline comments. So multiline comments are redundant.

### Strings
They start with a `"` and end with a `"`. Unlike in many languages, a normal string is also a multiline string 
A string won't stop being parsed if a newline is hit, it will continue until it sees a closing `"`.
Single quotes (`'`) cannot be used to represent strings.

**Rationale:** Having more than one syntax for strings is a waste of time.

### Numbers
Integers are signed integer numbers that are ideally the same size as the highest addressable size in the system, usually `intptr_t` or `isize` in various languages.
Floating point numbers are numbers that are preferrably a 64-bit float, usually `double` or `f64` in various languages.

Number literals support number separators using `_`. So `100000` and `100_000` are both valid and mean the same thing.
They can also be represented in binary, octal and hexadecimal form through `0b`, `0o` and `0x` respectively.

### Arrays
Arrays cannot contain more than one type. They're homogenous.
The first item in the array determines what the type of the array is.
If there's an array `[2, "value", { name = "name" }]`, the first item (`2`) is an int. Therefore the array is of type int and
cannot store any other type. So the example array here should be an error.

**Rationale:** Homogenous types are consistent and easy to use in both static and dynamic typed languages.
Accepting any type in arrays would not map well to statically typed languages.

### Blocks
Just like with arrays, the first key's type encountered in a block determines the type for all keys in the block.
Values can be of any type.

Fields and strings are both considered to be string keys, therefore they can be used interchangeably as keys.

So:
```c
block = {
  something = true,  
  "something else" = [1, 2, 3],
}
```
and
```c
block = {
  0 = "something",
  1 = "something else",
}
```
Are both valid.

But
```c
block = {
  something = true,
  5 = [1, 2, 3],
}
```
Results in an error.

**Rationale:** Blocks are basically the same as a hashmap, so the keys have to be of the same type.
There's no real difference between a field identifier like `my_val` and a string `"my_val"` when passed to a hash function,
therefore, the distinction doesn't matter.

### Dotted fields
A key with a dot is considered to be a field access. This means that `key.subkey = "value"` is syntax sugar for:
```c
key = {
  subkey = "value"
}
```

**Rationale**: In config files often configuration is deeply nested. Having this shorthand is useful to avoid clutter and it also saves space and extra typing.
