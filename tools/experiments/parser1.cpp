// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/internal_parser.hpp"

#include <iostream>

#include "quick/debug.hpp"
#include "quick/stl_utils.hpp"

#include "../../tests/samples/sample_internal_parser_rules.hpp"


using aparse::InternalParser;
using aparse::InternalParserBuilder;

int main()  {
  aparse::InternalParser<test::Grammar3ParserRules> p3;
  aparse::InternalParserBuilder<test::Grammar3ParserRules>().Build(&p3);
  {
    p3.Reset();
    // [NUM, NUM]
    p3.Feed({0, 6, 4, 6, 1});
    p3.End();
    cout << p3.GetParseTree().DebugString() << endl;
  }
  cout << p3.Export() << endl;
  return 0;
}

