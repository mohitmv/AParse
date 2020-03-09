// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_SRC_PARSE_CHAR_REGEX_HPP_
#define APARSE_SRC_PARSE_CHAR_REGEX_HPP_

#include <string>

#include "src/parse_char_regex_rules.hpp"

namespace aparse {

struct ParseCharRegex {
  using ParserType = Parser;
  static Regex Parse(const string& input);
  static bool Parse(const string& input, Regex* output, Error* error);
};

}  // namespace aparse

#endif  //  APARSE_SRC_PARSE_CHAR_REGEX_HPP_
