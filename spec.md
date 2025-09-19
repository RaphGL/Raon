# RAON Specification

```py
entry = ({field} "=" {value})*
block = "{" {entry}* "}" 
field = [A-z | 0-9 | "-" | "_"]*
value = ({string} | {bool} | {int} | {block} | {array}) ("," | "\n")
string = [\W\d]* # a string can be any valid unicode
bool = "true" | "false"
int = "-"? 0-9*
array = "[" {value}*  "]"
```

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
cannot store any other type. So this should be an error.

**Rationale:** Homogenous types make for more predictable input, it's easy for the machine and for users to understand.
It also makes it much easier to simply copy the array directly to any language's builtin array type. If it were dynamic
it would be harder to do this in statically typed languages.

### Dotted fields
TODO
