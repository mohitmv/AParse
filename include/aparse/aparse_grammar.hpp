// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)


#ifndef _APARSE_APARSE_GRAMMAR_HPP_
#define _APARSE_APARSE_GRAMMAR_HPP_

#include <utility>
#include <vector>
#include <string>

#include "aparse/common_headers.hpp"
#include "aparse/regex.hpp"

namespace aparse {
struct AParseGrammar {
  bool Validate() const;
  std::string DebugString() const;
  std::size_t GetHash() const;

  int alphabet_size;
  std::vector<pair<int, Regex>> rules;
  std::vector<pair<Alphabet, Alphabet>> branching_alphabets;
  int main_non_terminal;
  // Map(Alphabet -> String meaning of the alphabet)
  std::unordered_map<Alphabet, string> alphabet_map;
};
}  // namespace aparse

#endif  // _APARSE_APARSE_GRAMMAR_HPP_
