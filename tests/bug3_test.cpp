// Copyright: 2019 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <iostream>

#include "quick/debug.hpp"
#include "gtest/gtest.h"

#include "aparse/parser.hpp"
#include "aparse/parser_builder.hpp"

TEST(InternalException, Basic) {
  struct S {};
  using Rule = aparse::ParserGrammar::Rule;
  using ParserScope = aparse::ParserScopeBase<S>;
  aparse::ParserGrammar grammar;
  grammar.main_non_terminal = "main";
  grammar.string_to_alphabet_map = {
                                  {"constant", 0},
                                  {"not", 1},
                                  {"not_in", 2},
                                  {"in", 3},
                                  {",", 4},
                                };
  grammar.rules = {
    Rule("main ::= constexpr ((in|not_in|not|in|not_in) constexpr)*"),
    Rule("constexpr ::= constant "),
  };
  aparse::Parser parser;
  aparse::ParserBuilder parser_builder;
  parser_builder.Build(grammar, &parser);
  auto parser_i = parser.CreateInstance();
  // constant in constant not_in constant not constant
  parser_i.FeedOrDie({0, 3, 0, 2, 0, 1, 0});
  parser_i.EndOrDie();
  // std::cout << parser_i.core_tree.DebugString() << std::endl;
  S s;
  ParserScope scope;
  parser_i.CreateSyntaxTree(&scope, &s);
}

