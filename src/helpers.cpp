// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/helpers.hpp"

#include <string>
#include "src/utils.hpp"

namespace aparse {
namespace helpers {

std::string GetAlphabetString(const std::string& s) {
  if (utils::IsLiteralName(s)) {
    return s;
  } else {
    return string("'") + s + "'";
  }
}

}  // namespace helpers
}  // namespace aparse
