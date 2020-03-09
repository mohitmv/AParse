// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_UTILS_ASSERT_HPP_
#define APARSE_UTILS_ASSERT_HPP_

#include "aparse/utils/very_common_headers.hpp"

#define APARSE_GET_MACRO(A1, A2, A3, A4, NAME, ...) NAME

#define APARSE_DEBUG_ASSERT_1(condition)  \
  if (APARSE_DEBUG_FLAG) {                \
    assert(condition);                    \
  }

#define APARSE_DEBUG_ASSERT_2(condition, error_message)    \
  if (APARSE_DEBUG_FLAG) {                                 \
    if (not(condition)) {                                  \
      std::cerr << error_message                           \
                << "\n[" << __FILE__ << ":"                \
                << __LINE__ << " " << __func__             \
                << "] Assertion `"                         \
                << #condition << "` Failed !"              \
                << std::endl;                              \
      std::abort();                                        \
    }                                                      \
  }

#define APARSE_ASSERT_1(condition)                        \
  APARSE_ASSERT_2(condition, "")

#define APARSE_ASSERT_2(condition, error_message)          \
  {                                                        \
    if (not(condition)) {                                  \
      std::cerr << error_message                           \
                << "\n[" << __FILE__ << ":"                \
                << __LINE__ << " " << __func__             \
                << "] Assertion `"                         \
                << #condition << "` Failed !"              \
                << std::endl;                              \
      std::abort();                                        \
    }                                                      \
  }

#define APARSE_DEBUG_ASSERT(...)                       \
  APARSE_GET_MACRO(__VA_ARGS__, UNDEFINED, UNDEFINED,  \
            APARSE_DEBUG_ASSERT_2,                     \
            APARSE_DEBUG_ASSERT_1,                     \
            UNDEFINED)(__VA_ARGS__)

#define APARSE_ASSERT(...)                             \
  APARSE_GET_MACRO(__VA_ARGS__, UNDEFINED, UNDEFINED,  \
                    APARSE_ASSERT_2,                   \
                    APARSE_ASSERT_1,                   \
                    UNDEFINED)(__VA_ARGS__)

#endif  // APARSE_UTILS_ASSERT_HPP_
