

## Available Variables For Parser

- `$<NonTerminalName>.Value()`
- `$<NonTerminalName>.ValueList()`
- `$<NonTerminalName>.ValueList(int index)`
- `$<NonTerminalName>.Exists()`

- `$<TerminalName>.GetAlphabet()`
- `$<TerminalName>.Index()`
- `$<TerminalName>.Exists()`
- `$<TerminalName>.IndexList()`
- `$<TerminalName>.IndexList(int index)`

- `$<index>` 

- `$.Range()`

- `$.Value()`
- `$.ValueList()`
- `$.ValueList(int index)`

- `$.GetAlphabet()`
- `$.Index()`
- `$.IndexList()`
- `$.IndexList(int index)`
- `$.AlphabetList()`
- `$.AlphabetList(int index)`



1\. `$<NonTerminalName>.Value()`
----------

- Type: `NodeType`
- Refers to the returned value(parsed-node) by the rule defining `<NonTerminalName>`.
- It's undefined if `$<NonTerminalName>.Exists()` is false.

Consider the Example:
`JsonPair ::= STRING ':' JsonExpr `
and it's parsing action:
```cpp
auto output = Json(Json::DICT_TYPE);
output.dict[GetString($STRING.Index())] = std::move($JsonExpr.Value()));
return output;
```
Here `$JsonExpr.Value()` refers to the value (syntax tree node), parsed by `JsonExpr` non-terminal.


2\. `$<NonTerminalName>.ValueList()`
----------

- Type: `std::vector<NodeType>`
- Refers to the list of returned values (parsed-node) by `$<NonTerminalName>`.
- If RHS expression of a rule contains multiple occurance of a non-terminal or if single occurance of a non-terminal is repeated using klenee operation (`*` or `+`), then `$<NonTerminalName>.ValueList()` stores the list of values returned by each occurance of non-terminal.
- `$<NonTerminalName>.Value()` is an alias for `$<NonTerminalName>.ValueList()[0]`

Example:
`JsonList ::= '[' (Json ',')* Json? ']'`
and it's parsing action:
```cpp
return Json(Json::LIST_TYPE, std::move($Json.ValueList()));
```
Here `$Json.ValueList()` refers to list of json children.



3\. `$<NonTerminalName>.ValueList(int index)`
----------

- Type: `NodeType`
- Alias for `$<NonTerminalName>.ValueList()[index]`

4\. `$<NonTerminalName>.Exists()`
----------

- Type: `bool`
- True iff underlying `<NonTerminalName>` is present.
- Alias for `$<NonTerminalName>.ValueList().size() > 0`

Consider the example:
`Json ::= JsonDict | TRUE_JSON | FALSE_JSON`
and it's parsing action:
```cpp
if ($JsonDict.Exists()) {
  return $JsonDict.Value();
} else {
  return Json(Json::BOOL_TYPE, $.GetAlphabet() == TRUE_JSON);
}
```
Here `$JsonDict.Exists()` checks the presence of JsonDict non-terminal.


5\. `$<TerminalName>.GetAlphabet()`
----------

- Type: `int`

8\. `$<TerminalName>.Index()`
----------

- Type: `int`
- Index of the `TerminalName` terminal in token sequence.

9\. `$<TerminalName>.Exists()`
----------

- Type: `bool`
- Refers to the

10\. `$<TerminalName>.IndexList()`
----------

- Type: `vector<int>`
- Refers to the

11\. `$<TerminalName>.IndexList(int index)`
----------

- Type: `int`
- Refers to the


12\. `$<index>`
----------

- Alias to corrosponding `$<NonTerminalName>` or `$<TerminalName>`
- Ex: `$0`, `$1`, `$2`, ...


13\. `$.Range()`
----------

- Type: `pair<int, int>`
- Refers to the

14\. `$.Value()`
----------

- Type: `NodeType`
- Refers to the

15\. `$.GetAlphabet()`
----------

- Type: `int`
- Refers to the

16\. `$.Index()`
----------

- Type: `int`
- Refers to the

17\. `$.ValueList()`
----------

- Type: `vector<NodeType>`
- Refers to the

18\. `$.ValueList(int index)`
----------

- Type: `NodeType`
- Refers to the

19\. `$.IndexList()`
----------

- Type: `vector<int>`
- Refers to the

20\. `$.IndexList(int index)`
----------

- Type: `int`
- Refers to the




## Available Variables For Lexer

- `$.Range()`: `pair<int, int>`
- `$.TokenString()`: `std::string`


