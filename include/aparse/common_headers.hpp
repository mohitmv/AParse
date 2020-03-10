// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

/** Include this header for the commonly used stuff. It includes:
 *  1. very common STL containers.
 *  2. AParse specific terms and typenames. (Eg: using Alphabet = int32_t)
 *  3. <aparse/assert.hpp> for aparse specific assertions. */

#ifndef APARSE_COMMON_HEADERS_HPP_
#define APARSE_COMMON_HEADERS_HPP_

#include <assert.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <limits>
#include <iostream>

#include "aparse/utils/very_common_headers.hpp"
#include "aparse/utils/assert.hpp"

namespace aparse {
using std::pair;
using std::make_pair;
using std::unordered_map;
using std::unordered_set;
using std::set;
using std::map;
using std::vector;
using std::string;
using std::make_tuple;
using std::cout;
using std::endl;
}  // namespace aparse

#endif  // APARSE_COMMON_HEADERS_HPP_
