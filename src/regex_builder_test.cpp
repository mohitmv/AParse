// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/regex_builder.hpp"

#include <quick/debug.hpp>
#include "gtest/gtest.h"

using aparse::RegexBuilderObject;
using std::cout;
using std::endl;
using std::string;
using std::unordered_map;

TEST(RegexBuilderTest, Basic) {
  using R = RegexBuilderObject;
  auto e1 = ((R("Expr") + (R("+") | R("-") | R())).Kstar().SetLabel(111) +
             R("Expr"));
  unordered_map<int, string> alphabet_map;
  auto r = e1.BuildRegex(&alphabet_map);
  EXPECT_EQ("((((2 (0 | 1 | EPSILON)))*)[111] 2)", r.DebugString());
  EXPECT_EQ("((((Expr ('+' | '-' | EPSILON)))*)[111] Expr)",
            r.DebugString(alphabet_map));
}

TEST(RegexBuilderTest, Example1) {
  using R = RegexBuilderObject;
  auto rb = (   R("NUM")  // NOLINT
             |  R("STRING")
             |  R("BOOL")
             |  R("NULL")
             | (R("[") + R("JsonList") + R("]"))
             | (R("{") + R("JsonDict") + R("}")));
  unordered_map<int, string> alphabet_map;
  auto r = rb.BuildRegex(&alphabet_map);
  EXPECT_EQ("(NUM | STRING | BOOL | NULL | ('[' JsonList ']') "
            "| ('{' JsonDict '}'))", r.DebugString(alphabet_map));
  EXPECT_EQ("(4 | 5 | 0 | 3 | (6 2 7) | (8 1 9))", r.DebugString());
}
