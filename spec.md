```py
entry = ({field} "=" {value})*
block = "{" {entry}* "}" 
field = [A-z | 0-9 | "-" | "_"]*
value = ({string} | {bool} | {int} | {block} | {array}) ("," | "\n")
string = [\W\d]* # a string can be any valid unicode
bool = "true" | "false"
int = 0-9*
array = "[" {value}*  "]"
```
