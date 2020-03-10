// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/parse_char_regex.hpp"
#include "src/internal_parser_builder.hpp"

#include <quick/time.hpp>
#include <quick/debug.hpp>

namespace aparse {

using uchar = unsigned char;

Regex ParseCharRegex::Parse(const string& input) {
  Regex output;
  Error error;
  if (not Parse(input, &output, &error)) {
    throw error;
  }
  return output;
}

bool ParseCharRegex::Parse(const string& input, Regex* output, Error* error) {
  static ParserType char_regex_parser;
  if (not char_regex_parser.IsFinalized()) {
    auto p_rules = CharRegexParserRules();
    InternalParserBuilder::Build(p_rules.first,
                                 p_rules.second,
                                 &char_regex_parser);
  }
  auto parser = char_regex_parser.CreateInstance();
  for (char c : input) {
    if (not parser.Feed(uchar(c), error)) {
      return false;
    }
  }
  if (not parser.End(error)) {
    return false;
  }
  CharRegexParserScope scope;
  scope.content = &input;
  parser.CreateSyntaxTree(&scope, output);
  return true;
}

}  // namespace aparse
