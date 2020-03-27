// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_SRC_HELPERS_HPP_
#define APARSE_SRC_HELPERS_HPP_

#include <string>

namespace aparse {
namespace helpers {

/** Given the alphabet-string, it returns the same with a "'" quotation
 *  applied if the input string was not alphanumeric. */
std::string GetAlphabetString(const std::string& s);

}  // namespace helpers
}  // namespace aparse

#endif  // APARSE_SRC_HELPERS_HPP_
