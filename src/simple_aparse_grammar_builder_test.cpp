// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/simple_aparse_grammar_builder.hpp"

#include <quick/debug.hpp>
#include "gtest/gtest.h"

using aparse::RegexBuilderObject;
using std::cout;
using std::endl;
using std::vector;

TEST(SimpleAParseGrammarBuilderTest, Basic) {
  struct Node {
    vector<Node> children;
  };
  using GrammarBuilder = aparse::SimpleAParseGrammarBuilder;
  using R = aparse::RegexBuilderObject;
  using ParserScope = aparse::ParserScopeBase<Node>;
  GrammarBuilder grammar_builder;
  grammar_builder.branching_alphabets = {{"(", ")"}};
  grammar_builder.main_non_terminal = "main";
  grammar_builder.rules = {
    GrammarBuilder::Rule("main", (R("(") + R("main") + R(")")).Kstar())
      .Action([](ParserScope* scope, Node* output) {
        output->children = std::move(scope->ValueList());
      })
  };
  grammar_builder.Build();
  auto& grammar = grammar_builder.aparse_grammar;
  grammar.Validate();
  EXPECT_EQ(2, grammar.alphabet_size);
  EXPECT_EQ(2, grammar.main_non_terminal);
  EXPECT_EQ(1, grammar.rules.size());
  EXPECT_EQ(2, grammar.rules.at(0).first);
  EXPECT_EQ("(('(' main ')'))*",
            grammar.rules.at(0).second.DebugString(grammar.alphabet_map));
}

