#include "aparse/advance_lexer.hpp"

#include <iostream>
#include "src/parse_char_regex.hpp"

using std::cout;
using std::endl;

namespace aparse {
namespace helpers {

bool ParseCharRegexFunc(const string& input, Regex* output, Error* error) {
  return ParseCharRegex::Parse(input, output, error);
}

}  // namespace helpers
}  // namespace aparse

