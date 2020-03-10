// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/aparse_grammar.hpp"

#include <iostream>

#include "gtest/gtest.h"

using aparse::AParseGrammar;
using aparse::Regex;

TEST(AParseGrammar, Basic) {
  AParseGrammar g;
  g.alphabet_size = 4;
  EXPECT_ANY_THROW(g.Validate());
  Regex r1(Regex::UNION);
  g.rules = {
    {4, Regex(1)},
    {5, Regex(2)},
    {6, Regex(Regex::EPSILON)},
  };
  g.main_non_terminal = 5;
  g.Validate();
  EXPECT_NO_THROW(g.Validate());
  EXPECT_GT(g.DebugString().size(), 0);
}
