// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_SRC_REGEX_BUILDER_HELPERS_HPP_
#define APARSE_SRC_REGEX_BUILDER_HELPERS_HPP_

#include <vector>
#include <unordered_set>
#include <utility>
#include <string>

#include "aparse/common_headers.hpp"
#include "aparse/regex.hpp"
#include "quick/debug_stream_decl.hpp"

namespace aparse {
namespace helpers {
// Given a regex, it reduce it to follow minimal-regex-property.
Regex ReduceRegex(const Regex input);

bool IsAllRegexChildrenAtomic(const Regex& regex);

std::unordered_set<Alphabet> GetRegexAtoms(const Regex& r);

// returns Regex(Regex::UNION, {a, a+1, a+2, ... b-1});
// a: inclusive, b: exclusive.
Regex RangeRegex(Alphabet a, Alphabet b);

Regex AllAlphabetExceptSomeRegex(const std::vector<Alphabet>& input,
                                        int alphabet_size);

inline Regex AnyAlphabetRegex(int alphabet_size) {
  return RangeRegex(0, alphabet_size);
}

// --------------------- Regex Building Shortcuts ---------------------

// R: Range
// E: EPSILON
// C: CONCAT
// U: UNION
// A: ATOMIC
// KP: KLEENE_PLUS
// KS: KLEENE_STAR
// SU: String UNION
// SC: String CONCAT

inline Regex R(Alphabet a, Alphabet b) {return RangeRegex(a, b);}
inline Regex E() { return Regex(Regex::EPSILON);}
inline Regex C(std::vector<Regex>&& u) {return Regex(Regex::CONCAT, u);}
inline Regex U(std::vector<Regex>&& u) {return Regex(Regex::UNION, u);}
inline Regex KP(Regex&& r) {return Regex(Regex::KPLUS, {r}); }
inline Regex KS(Regex&& r) {return Regex(Regex::KSTAR, {r}); }
inline Regex A(char c) {return Regex(static_cast<unsigned char>(c));}
inline Regex A(int a) {return Regex(a);}
Regex SU(const string& s);
Regex SC(const string& s);

// ---------------- Regex Building Shortcuts Ends Here ----------------


}  // namespace helpers
}  // namespace aparse

#endif  // APARSE_SRC_REGEX_BUILDER_HELPERS_HPP_
