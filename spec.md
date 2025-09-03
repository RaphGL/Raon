```py
entry = ({field} "=" {value} ("," | "\n"))*
block = "{" {entry}* "}" 
field = [A-z | 0-9 | "-" | "_"]*
value = {string} | {bool} | {int} | {block}
string = [\W\d]* # a string can be any valid unicode
bool = "true" | "false"
int = 0-9*
```
