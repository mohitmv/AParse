// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_COMMON_HEADERS_HPP_
#define _APARSE_COMMON_HEADERS_HPP_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <assert.h>
#include <limits>
#include <iostream>
#include "aparse/very_common_headers.hpp"

#ifndef _APARSE_DEBUG_FLAG
#define _APARSE_DEBUG_FLAG true
#endif

// #if _APARSE_DEBUG_FLAG == true
// using namespace std;
// #endif

#define _APARSE_GET_MACRO(_1,_2,_3,_4,NAME,...) NAME

#define _APARSE_DEBUG_ASSERT_1(condition) \
  if (_APARSE_DEBUG_FLAG) {               \
    assert(condition);                    \
  }

#define _APARSE_DEBUG_ASSERT_2(condition, error_message)   \
  if (_APARSE_DEBUG_FLAG) {                                \
    bool _aXty6hu0 = (condition);                          \
    if(not _aXty6hu0) {                                    \
      std::cerr << error_message                           \
                << "\n[" << __FILE__ << ":"                \
                << __LINE__ << " " << __func__             \
                << "] Assertion `"                         \
                << #condition << "` Failed !"              \
                << std::endl;                              \
      exit(1);                                             \
    };                                                     \
  }

#define _APARSE_ASSERT_1(condition)                        \
  _APARSE_ASSERT_2(condition, "")

#define _APARSE_ASSERT_2(condition, error_message)         \
  {                                                        \
    if(not (condition)) {                                  \
      std::cerr << error_message                           \
                << "\n[" << __FILE__ << ":"                \
                << __LINE__ << " " << __func__             \
                << "] Assertion `"                         \
                << #condition << "` Failed !"              \
                << std::endl;                              \
      exit(1);                                             \
    };                                                     \
  }

#define _APARSE_DEBUG_ASSERT(...)                       \
 _APARSE_GET_MACRO(__VA_ARGS__, UNDEFINED, UNDEFINED,   \
           _APARSE_DEBUG_ASSERT_2,                      \
           _APARSE_DEBUG_ASSERT_1,                      \
           UNDEFINED)(__VA_ARGS__)

#define _APARSE_ASSERT(...)                             \
  _APARSE_GET_MACRO(__VA_ARGS__, UNDEFINED, UNDEFINED,  \
                    _APARSE_ASSERT_2,                   \
                    _APARSE_ASSERT_1,                   \
                    UNDEFINED)(__VA_ARGS__)



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
}


#endif  // _APARSE_COMMON_HEADERS_HPP_
