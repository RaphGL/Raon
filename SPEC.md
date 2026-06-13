# RAON Specification

```grammar
file = (entry separator)*
entry  = key "=" value
value  = (string | bool | int | block | array)
block  = "{" (entry separator)* "}" 
array  = "[" (value separator)* "]"
key = field | int | string
field  = (A-z | 0-9 | "-" | "_")*
string = "\"" [\W\d]* "\"" # a string can be any valid unicode
bool   = "true" | "false"
int    = "-"? 0-9*
separator = "," | "\n" | end_char

# end_char here means the ending character for arrays or blocks
# this is so that arrays or blocks can be inlined without trailing commas
```

Grammar syntax:
- `=`: starts a grammar point declaration.
- `*`: the previous token is repeated zero or more times. 
- `?`: the previous token is repeated 0 or 1 time.
- `""`: anything in quotes is a literal.
- `()`: used for grouping, for example `(num "+" num)*` means that this will happen 0 or more times.
- `|`: used to denote that either the left or the right side will happen, for example `string | bool` means that this will either be
whatever is defined in the string grammar point or in the bool grammar point.

### Primitive types
There's only 5 types in Raon:
- int: represents signed integers, it should ideally be the highest indexable size in the system (`intptr_t` in C or `isize` in many other languages)
- bool: represents a boolean type
- string: any valid language unicode character
- array: an array of items of one of the types
- block: an associative array whose values are of the other types

**Rationale:** Floats are not part of the language because they're complicated, I don't really want to make Raon harder to implement.
People would also start asking for scientific notation and whatnot. If you really really need to use other formats like floats, scienfitic
notation, dates and whatnot, just use a string. A string can be anything you like, you can then parse/validate it in your program.

### Comments
They start with a `#` and stop once a newline is found.

**Rationale:** an inline comment can be used to write multiline comments. So multiline comments are effectively useless.

### Strings
They start with a `"` and end with a `"`. Unlike in many languages, a normal string is also a multiline string 
A string won't stop being parsed if a newline is hit, it will continue until the end of the file or until it sees a `"`.
Single quotes (`'`) cannot be used to represent strings.

**Rationale:** Having more than one syntax for strings is a waste of time.

### Arrays
Arrays cannot contain more than one type. They're homogenous.
The first item in the array determines what the type of the array is.
If there's an array `[2, "value", { name = "name" }]`, the first item (`2`) is an int. Therefore the array is of type int and
cannot store any other type. So the example array here should be an error.

**Rationale:** Homogenous types are consistent and easy to use in both static and dynamic typed languages.
Accepting any type in arrays would not map well to statically typed languages.

### Blocks
Just like with arrays, the first type encountered in a block's key determines the type for all keys in the block.

Fields and strings are both considered to be string keys, therefore they can be used together.

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
