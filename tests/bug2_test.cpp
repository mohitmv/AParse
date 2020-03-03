
TEST(TestBuilderVariables, DISABLED_Basic) {
  struct S {};
  struct ParserRules2: aparse::AdvanceParserRulesBase<S> {
    void Init() {
      this->string_to_alphabet_map = {
                                      {"const", 0},
                                      {"{", 1},
                                      {"}", 2},
                                      {"in", 3},
                                      {",", 4},
                                    };
      // This test works if `branching_alphabets` are removed.
      this->branching_alphabets = {{1, 2}};
      this->main_non_terminal = "main";
      this->rules = {
        Rule("main ::= constexpr (in constexpr_list)*")
          .Action([](ParsingScope* scope, S* output) {
            EXPECT_EQ(scope->ValueList().size(),
                      scope->GetAlphabetList().size()+1);
          }),
        Rule("constexpr ::= const "),
        Rule("constexpr_list ::= ('{' constexpr_list_expr '}') "),
        Rule("constexpr_list_expr ::= ((constexpr ',')* constexpr)? ")
      };
    }
  };
  aparse::AdvanceParser<ParserRules2> parser;
  aparse::AdvanceParserBuilder<ParserRules2>().Build(&parser);
  parser.Feed({0, 3, 1, 0, 4, 0, 4, 0, 2});
  parser.End();
  cout << parser.core_tree.DebugString() << endl;
  S s;
  parser.CreateSyntaxTree(&s);
  exit(0);
}

