// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

/** There are two public macros:
 *  1. APARSE_ASSERT
 *     It calls std::abort when boolean condition fails.
 *     Usage:
 *       APARSE_ASSERT(x > 3)
 *       APARSE_ASSERT(x > 3, "x has to be greater than 3")
 *       APARSE_ASSERT(x > 3, "x = " << x << ", but expecting x > 3.")
 *     Second argument is optional. Second arg can have multiple printable
 *      objects streamed using '<<'. Second arg expr is placed in front of
 *      std::cerr
 *
 *  2. APARSE_DEBUG_ASSERT
 *     Same as APARSE_ASSERT when APARSE_DEBUG_FLAG=true
 *     Otherwise it's equivalent to nop.
 *     bool-expression used inside APARSE_DEBUG_ASSERT must be read-only
 *     expression. Currently there is no language level feature to enforce the
 *     restriction, so it's developer's responsibility to ensure that. BTW
 *     support for the read-only scope in C++ is being debated and expected to
 *     come soon in newer releases of C++ standard.
 *     [[DANGER]]: In non-debug-mode, APARSE_DEBUG_ASSERT is a nop instruction,
 *     i.e. it doesn't evaluate the bool-expression, which might have side
 *     effects. Eg: The instruction `APARSE_DEBUG_ASSERT(DoTask())`, which was
 *     expected to validate the return status of DoTask in debug mode, will end
 *     up introducing a bug because DoTask was not executed in production-mode.
 */
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
