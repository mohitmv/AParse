// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_SRC_PARSE_CHAR_REGEX_HPP_
#define APARSE_SRC_PARSE_CHAR_REGEX_HPP_

#include <string>

#include "src/parse_char_regex_rules.hpp"

namespace aparse {

/** This utility is used for converting a regex-string to Regex object.
 *  Regex strings are easy to express hence we allow the AParse's clients
 *  to express their regex in C++ string. If a string is not valid regex then
 *  this utility returns the detailed error.
 *  Learn more at `src/parse_char_regex_test.cpp` */
struct ParseCharRegex {
  using ParserType = Parser;
  static Regex Parse(const string& input);
  static bool Parse(const string& input, Regex* output, Error* error);
};

}  // namespace aparse

#endif  //  APARSE_SRC_PARSE_CHAR_REGEX_HPP_
