// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)


#ifndef APARSE_APARSE_GRAMMAR_HPP_
#define APARSE_APARSE_GRAMMAR_HPP_

#include <utility>
#include <vector>
#include <string>
#include <unordered_map>

#include "aparse/utils/very_common_headers.hpp"
#include "aparse/regex.hpp"

namespace aparse {

/** Learn more about definition of AParseGrammar at
 *  https://aparse.readthedocs.io */
struct AParseGrammar {
  bool Validate() const;
  std::string DebugString() const;
  std::size_t GetHash() const;

  int alphabet_size;
  std::vector<std::pair<int, Regex>> rules;
  std::vector<std::pair<Alphabet, Alphabet>> branching_alphabets;
  int main_non_terminal;
  // Map(Alphabet -> String meaning of the alphabet)
  std::unordered_map<Alphabet, std::string> alphabet_map;
};
}  // namespace aparse

#endif  // APARSE_APARSE_GRAMMAR_HPP_
