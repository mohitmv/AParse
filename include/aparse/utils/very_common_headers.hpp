// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

/** This header only file contains very commonly used definitions. It's used
 *  by almost all components of AParse. */
#ifndef APARSE_UTILS_VERY_COMMON_HEADERS_HPP_
#define APARSE_UTILS_VERY_COMMON_HEADERS_HPP_

namespace aparse {

/** Alphabets of a grammar are {0, 1, 2, ... alphabet_size-1} */
using Alphabet = int32_t;

/** EnclosedNonTerminal is a special kind of non-terminal, which is
 *  auto-generated in the preprocessing step of AParseGrammar.
 *  Learn more about the Enclosed-Non-Terminal at aparse.readthedocs.io in the
 *  section defining AParseGrammar */
using EnclosedNonTerminal = int32_t;

}  // namespace aparse

#ifndef APARSE_DEBUG_FLAG
#define APARSE_DEBUG_FLAG true
#endif

#endif  // APARSE_UTILS_VERY_COMMON_HEADERS_HPP_
