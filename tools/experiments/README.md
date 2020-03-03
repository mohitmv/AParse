





Public Interface of LexerBuilder and ParserBuilder:

  enum TokenType { STRING, NUMBER, LITERAL, NONE,
                   BRACKET_1_OPEN, BRACKET_1_CLOSE, BRACKET_2_OPEN,
                   BRACKET_2_CLOSE, BRACKET_3_OPEN, BRACKET_3_CLOSE,
                   SYMBOL_PLUS, SYMBOL_OR, SYMBOL_COLON, SYMBOL_EQUAL,
                   SEMI_COLON, SYMBOL_STAR, SYMBOL_DOT
                  };
  aparse::LexerBuilder<TokenType> lb;
  lb.Rules({
    lb.Rule( "[0-9]+",                 NUMBER),
    lb.Rule( "[a-zA-Z_][a-zA-Z0-9_]*", LITERAL),
    lb.Rule(literal, STRING)});
  auto lexer = lb.Build();  // constexpr
  pair<TokenType, pair<int, int>> tokens = lexer.Parse(string);


  aparse::ParserBuilder<SyntaxTreeNode> pb;
  pb.branching_alphabets = ({{BRACKET_1_OPEN, BRACKET_1_CLOSE}});
  if (true) {
    pb.alphabet_size = 50;
    pb.main_symbol2_ = non_terminal_1_id;
    pb.rules2 = (unordered_map<int, pair<Regex, Lambda>>){
      { non_terminal_1_id,
        { Regex1,
          [&](pb::RuleInput& input, SyntaxTreeNode* output) {
            *output = std::move(input.Get(1).value());
          }
        }
      }
      { non_terminal_2_id, { Regex2, Lambda2}}
    };
  } else {
    pb.terminals = ({{"(", BRACKET_1_OPEN},
                  {"LITERAL", LITERAL}});
    pb.main_symbol_ = "Main3"; // default is : "Main".
    pb.rules = {
      {"OredExpression = '(' (StaredExpression '|')* StaredExpression ')' ",
        [](pb::RuleInput& input, SyntaxTreeNode* output) {
          *output = std::move(input.Get(1).value());
        }
      }
    };
  }
  constexpr auto parser = pb.Build();  // constexpr

  for(auto& file: files) {
    SyntaxTreeNode file_parse_tree;
    parser.Parse(vector<Alphabet>(file_tokens), &file_parse_tree);
  }



