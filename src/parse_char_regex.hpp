// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_PARSE_CHAR_REGEX_HPP_
#define _APARSE_PARSE_CHAR_REGEX_HPP_

#include "src/parse_char_regex_rules.hpp"

namespace aparse {

struct ParseCharRegex {
  using ParserType = Parser;
  static Regex Parse(const string& input);
  static bool Parse(const string& input, Regex* output, Error* error);
};

}


#endif //  _APARSE_PARSE_CHAR_REGEX_HPP_
