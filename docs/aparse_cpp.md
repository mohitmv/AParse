How to use AParse C++
===================




To use AParse C++, you need to:
--------------------------

1. `#include "aparse/aparse.hpp"` in your header.
2. Define the grammar rules.
3. Declare some C++ type as a node of desired syntax-tree. It should be passed in templated argument `SyntaxTreeNode`.
4. Each rule should be attached with a parsing-action, which defines the mechanism of constructing node of syntax-tree, corrosponding to the non-terminal in the left hand side of rule.
5. Build the parser object from grammar rules. Export it into a file. Import it next time.
6. Every time you need to parse a string (stream of alphabets):
   - Create a parser-instance, feed the alphabets one by one into parser-instance.
   - Use `parser_instance.CreateSyntaxTree()` to get the parsed tree.



Let's Build Simple Arithmetic Expression Parser
-----------------------------------------------


Goal: Build a parser to parse simple arithmetic expressions, consisting of `+` , `*` , numbers and parentheses.
Ex:

- `5 + 6`,

- `56`

- `5 + 16 * 33 + ( 3 + (3 + 5 * 7 + 4) + 6)`,



```C++

struct Expression {
  enum Type {PLUS, STAR, NUMBER};
  Type type;
  vector<Expression> children;
  int value;
};

using MyParser = aparse::AdvanceParser<Expression>;
enum LexerTokens {PLUS, STAR, NUMBER, OPEN_B1, CLOSE_B1};

void BuildParser(MyParser* parser) {
  using Rule = MyParser::Grammar::Rule;
  using ParserScope = MyParser::ParserScope;
  MyParser::Grammar grammar;
  grammar.branching_alphabets = {{LexerTokens::OPEN_B1, LexerTokens::CLOSE_B1}};
  grammar.string_to_alphabet_map = {
                                    {"+", LexerTokens::PLUS},
                                    {"*", LexerTokens::STAR},
                                    {"NUMBER", LexerTokens::NUMBER},
                                    {"(", LexerTokens::OPEN_B1},
                                    {")", LexerTokens::CLOSE_B1},
                                  };
  grammar.main_non_terminal = "<main>";
  grammar.rules = {
      Rule("<main> ::= (<multiplied> '+')* <multiplied>")
        .Action([](ParserScope* scope, Expression* output) {
          if (scope->ValueList().size() == 1) {
            *output = std::move(scope->ValueList()[0]);
          } else {
            output->type = Expression::PLUS;
            output->children = std::move(scope->ValueList());
          }
        }),
      Rule("<number_expr> ::= NUMBER")
        .Action([](ParserScope* scope, Expression* output) {
          output->type = Expression::NUMBER;
          output->value = scope->AlphabetIndex();  // It's just index
        }),
      Rule("<atom> ::= <number_expr> | '(' <main> ')' ")
        .Action([](ParserScope* scope, Expression* output) {
          *output = std::move(scope->Value());
        }),
      Rule("<multiplied> ::= (<atom> '*')* <atom>")
        .Action([](ParserScope* scope, Expression* output) {
          if (scope->ValueList().size() == 1) {
            *output = std::move(scope->ValueList()[0]);
          } else {
            output->type = Expression::STAR;
            output->children = std::move(scope->ValueList());
          }
        })
  };
  parser->Build(grammar);
}

Expression Parse(const MyParser& parser, const vector<LexerTokens>& tokens) {
  auto parser_instance = parser.CreateInstance();
  for (auto c : tokens) {
    parser_instance.Feed(c);
  }
  parser_instance.End();
  return parser_instance.CreateSyntaxTree();
}

```



**Grammar**

As we can observe, the grammar below describes the language of simple arithmetic expressions, consisting of `+` , `*` , numbers and parentheses. The grammar respects  operator precedence as well.

```
<main> ::= (<multiplied> '+')* <multiplied>
<number_expr> ::= NUMBER
<atom> ::= <number_expr> | '(' <main> ')'
<multiplied> ::= (<atom> '*')* <atom>
```

**Parse Tree**

When alphabets are fed into parser, it creates parse tree, which defines the instances of grammar rules used for accepting a string.
The parse tree satisfies following properties:
1. leaf are terminals. non-leaf nodes are non-terminals. 
2. root is main-non-terminal.
3. For each non-leaf node of parse tree, the list of direct children is a rule-instance of the rule defining the non-terminal, corrosponding to the node. i.e. the list is regex-instance of the regex in RHS (rigt hand side) of the rule, corrosponding to the non-terminal of the node.
4. In-order traversal of parse tree, while ignoring the non-leaf nodes, reconstructs the original string.

**Parse Tree Examples**

*5 + 6*

```

             <main>
            /  |   \
           /   |    \
          /   "+"    \
         /            \
  <multiplied>     <multiplied>
       |                |
       |                |
    <atom>           <atom>
       |                |
       |                |
     NUMBER           NUMBER

```

- For the root node: [`<multiplied>`, `+`, `<multiplied>`] is a regex-instance of regex `(<multiplied> '+')* <multiplied>`
- Similarly for the `<multiplied>` node, [`<atom>`] is a regex-instance of regex `(<atom> '*')* <atom>`.


*56*

```
    <main> 
      |
      |
  <multiplied> 
      |
      |
    <atom> 
      |
      |
    NUMBER 

```


*4 + 3 * (5+2) + 46*


```
                   _<main>_
                __/ / | \  \__
             __/   /  |  \    \__
          __/     /   |   \      \__
         /      "+"   |   "+"       \
  <multiplied>        |           <multiplied>
      |               |                 |
      |          <multiplied>           |
    <atom>        /   |     \         <atom>
      |          /    |      \          |
      |      <atom>   |     <atom>      |
   NUMBER      |     "*"    /  |  \   NUMBER
               |           /   |   \
            NUMBER        /    |    \
                        "("  <main>  ")"
                            /  |   \
                           /   |    \
                          /    |     \
                         /    "+"     \
                 <multiplied>      <multiplied>
                     |                  |
                     |                  |
                   <atom>             <atom>
                     |                  |
                     |                  |
                  NUMBER              NUMBER



```

- For the root node, list [`<multiplied>`, `+`, `<multiplied>`, `+`, `<multiplied>`] is a rule-instance of the first rule. (`<main> ::= (<multiplied> '+')* <multiplied>`), i.e. it's regex-instance of RHS (`(<multiplied> '+')* <multiplied>`).



**Syntax Tree**
To construct syntax tree from the parse tree, each grammar rule (each non-terminal), should be attached with a lambda function (or equivalent), to define the mechanism of constructing a node of syntax-tree.

In the above example, there are 4 rules in the grammar. The first rule defines the non-terminal `<main>`. The action attached with this rule, defines the mechanism of constructing the node of syntax-tree corrosponding to non-terminal `<main>`. This action can safely assume that underlying node of syntax-tree are already constructed. i.e. the node corrosponding to non-terminals `<multiplied>` in right hand side is already constructed.
Internally, these actions, attached with each rule, are executed with parse-tree in bottom-up fashion to protect the above guarantee.

In this example, C++ data type `Expression` is the node type of syntax tree (`SyntaxTreeNode`).

**Parse("5 + 6", ...)**

```

          "+"
        /     \
       /       \
      5         6

```

**Parse("4 + 3 * (5+2) + 46", ...)**

```

                "+"
               / | \
              /  |  \
             4   |   46
                "*"
               /  \
              /    \
             3     "+"
                  /   \
                 /     \
                5       2

```

**Parse("5 + 16 * 33 + ( 3 + (3 + 5 * 7 + 4) + 6)", ...)**

```

        "+"
      /  |  \
     5  "*"    \
        /  \     \
       16   33   "+"
                / | \
               /  |   \
              3  "+"   6
                / |  \
              3  "*"   \
                /   \   4
               5     7

```

Note: In these examples, it’s assumed that lexing (tokenization) is already done before calling the `Parse` method. `Lexer Tokens` are the Alphabets of parser.

Each parsing rule is attached with a `Action`, which is a lambda function of following input/output interface : `std::function<void(ParserScope* scope, Expression* output)>`. It is used for defining the mechanism of syntax-tree-node construction, corresponding to parsing rule.

1. (ParserScope* scope) : Parser Scope have various APIs/variables, which can be used in the mechanism of syntax-tree-node construction.
Ex: `ValueList( )` is one such API, which returns `vector<Expression>&`, a list of syntax tree nodes, corresponding to child non-terminal of that rule. Basically, syntax-tree-node construction mechanism can assume that child syntax-tree-nodes are already constructed (due to `Action` attached with another rule, (The rule, defining the child non-terminal)).

2. (Expression* output) => Syntax tree node, being constructed from this lambda.


In the example above, there are 4 rules in the grammar of arithmetic expression. The first rule defines the non-terminal `<main>` using other non-terminals `<multiplied>`and terminals `+`.
The action attached with this rule, defines the mechanism of constructing the node of syntax-tree corrosponding to non-terminal `<main>`. This action can safely assume that underlying node of syntax-tree are already constructed. i.e. the node corrosponding to non-terminal `<multiplied>` is already constructed. The parsing action for non-terminal `<main>` is defined in this way: 
  - if there is exactly 1 children, then do nothing and make it output.
  - if there are multiple children, then output must be an expression of type `Expression::PLUS`, with children = list of all the nodes, which are already constructed for non-terminal `<multiplied>`.



## What is Parser Scope

ParserScope exposes helper APIs to access the nodes of syntax-tree, corrosponding to non-terminals in right hand side of a rule, which are already constructed.
ex: `ParserScope::ValueList()` returns a `vector<SyntaxTreeNode>&` object, which is list of syntax-tree-node corrosponding to all the non-terminals, occuring in right hand side, preserving the order of occurance.
The rule `<main> ::= (<multiplied> '+')* <multiplied>`, can be matched to any of 

`<multiplied>`

`<multiplied> '+' <multiplied>`

`<multiplied> '+' <multiplied> '+' <multiplied>`

`<multiplied> '+' <multiplied> '+' <multiplied> '+' <multiplied>`
....

These matches are called rule-instance.

Hence output of `ValueList()` can be a vector of size 1, 2, 3, ...


**ParserScope General Rules:**

1. Output of `ValueList()` will be different, when used in the parsing action for different grammar rule or different instance of a grammar rule. However in the scope of parsing action for a particular instance of a grammar rule, result of `ValueList()` will remain same without any additional complexity (because it returns a reference to alredy computed object). However inside the body of an action, mechanism of constructing a node might override it, for efficient operations (ex: `std::move` or `node.swap()` etc.). In such cases, next call to `ValueList()` will return the same vector object, reflecting the C++ defined changes, due to override actions (ex: std::move etc.).
2. Any legal write-operation on the objects-references returned by ParserScope APIs, will not have any side effect outside the scope of a parsing action, because syntax-tree is a tree, every node have at most one parent, hence underlying nodes (sub-tree) won't be served to any other parsing rule or any other instane of same parsing rule. Feel free to abuse the object reference returned by `ValueList()` in whatever way you want, as long as it has well defined destructor to take care of it. At the end everything will be destructed except what is constructed by main rule. (which might have stored the other nodes in it's children).
3. These rules are applicable for all other ParserScope APIs as well.


**ParserScope APIs**

1. `ValueList()`: returns the list of syntax-tree-node corrosponding to all the non-terminals occuring in the instance of rule, at right hand side, preserving the order of occurance.
2. `GetAlphabetList()`: returns the list of all the alphabets (terminals), occuring in the instance of rule, at right hand side, preserving the order of occurence.
return type : `vector<Alphabet>` (where Alphabet is an alias to int).
3. `AlphabetIndexList()`: returns the list of index-in-the-stream of all the alphabets (terminals), occuring in the instance of rule, at right hand side, preserving the order of occurence.
return type: `vector<int>`
`index-in-the-stream` : while feeding the alphabet to parser one-by-one, the first alphabet will have index-0, second one will have index-1, and so on...

4. `Value()`: referes to `ValueList()[0]`.
              it's undefined when `ValueList().size() == 0`, hence should not be accessed.
5. `GetAlphabet()` referes to `GetAlphabetList()[0]`.
6. `AlphabetIndex()` referes to `AlphabetIndexList()[0]`.

7. `ValueList(int index)`: Let P is the non-terminal, present the at the `index`'th term in the right hand side of rule, then it returns an object of type `vector<SyntaxTreeNode>&`, which is list of syntax-tree-node corrosponding to non-terminals P's, occuring in right hand side, preserving the order of occurance.
`ValueList()` returns a mix list, however `ValueList(int index)`, returns a filtered list for the specific non-terminal.
Note the in the rule above `<main> ::= (<multiplied> '+')* <multiplied>`, output of `ValueList()`, `ValueList(0)` and `ValueList(2)` are all same.
In the rule `<expr> ::= (<list> | NUMBER | STRING | <dict>)+`, 
Let output of `ValueList()` looks like: [`@expr0:list`, `@expr1:dict`, `@expr2:dict`, `@expr3:list`, `@expr4:dict`], then output 
of `ValueList(0)` will look like: [`@expr0:list`, `@expr3:list`, `@expr4:dict`]
output of `ValueList(3)` will look like: [`@expr1:dict`, `@expr2: dict`]

8. `Value(int index)`: refers to ValueList(index)[0];

9. `AlphabetIndexList(int index)` : Let T is the terminal, present at the `index`'th term in the right hand side of rule, then `AlphabetIndexList(int index)` returns an object of type `vector<Alphabet>`, which is list of index-in-the-stream of terminals T, occuring in the instance of rule.

10. `AlphabetIndex(int index)`: refers to `AlphabetIndexList(index)[0]`

11. `Exists(int index)`: checks if a terminal or non-terminal present at `index`'th term exists in the instance of rule.
    returns bool;





