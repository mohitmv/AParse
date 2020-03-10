// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_SRC_REGEX_BUILDER_HPP_
#define APARSE_SRC_REGEX_BUILDER_HPP_

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <string>

#include "quick/byte_stream.hpp"
#include "quick/unordered_map.hpp"
#include "quick/unordered_set.hpp"
#include "quick/debug_stream_decl.hpp"

#include "aparse/regex.hpp"
#include "src/utils.hpp"

namespace aparse {

/** RegexBuilderObject is a test utility. It's structure is similar to Regex.
 *  The only difference is RegexBuilderObject accepts string-alphabets, which
 *  makes it easy to create a Regex object. Regex expects the alphabets in
 *  integer form. Which is good for latency and performance but difficult for
 *  developers to understand the meanings of alphabets. This utility allows
 *  developers to create their regex object using string alphabets, which is
 *  later converted to original Regex object. This utility returns the
 *  alphabet-to-string conversion map as well. */
struct RegexBuilderObject {
  using RegexType = Regex::RegexType;
  RegexBuilderObject() {}
  explicit RegexBuilderObject(RegexType regex_type): type(regex_type) {}
  explicit RegexBuilderObject(Alphabet alphabet): type(Regex::ATOMIC),
                                         alphabet(alphabet) {}
  explicit RegexBuilderObject(const std::string& a): type(Regex::ATOMIC),
                                            alphabet_string(a) {}
  RegexBuilderObject(RegexType regex_type,
                     const std::vector<RegexBuilderObject>& children)
       : type(regex_type),
         children(children) {}
  RegexBuilderObject(RegexType regex_type,
                     std::vector<RegexBuilderObject>&& children)
     : type(regex_type),
       children(std::move(children)) {}

  void DebugStream(qk::DebugStream& ds) const;  // NOLINT

  RegexBuilderObject& operator+(const RegexBuilderObject& other);
  RegexBuilderObject& operator|(const RegexBuilderObject& other);
  RegexBuilderObject& Kplus();
  RegexBuilderObject& Kstar();
  RegexBuilderObject& Optional();
  RegexBuilderObject& SetLabel(int label);

  void GetSymbols(std::unordered_set<std::string>* output) const;
  std::unordered_set<std::string> GetSymbols() const;

  using StringMap = std::unordered_map<std::string, int>;
  using AlphabetMap = std::unordered_map<int, std::string>;
  void BuildRegex(const StringMap& symbols, Regex* output) const;
  Regex BuildRegex(const StringMap& symbols) const;
  void BuildRegex(AlphabetMap* alphabet_map, Regex* output) const;
  Regex BuildRegex(AlphabetMap* alphabet_map) const;
  Regex BuildRegex() const;

  RegexType type = Regex::EPSILON;
  Alphabet alphabet = 0;  // used only when children.size() == 0
  std::string alphabet_string;
  std::vector<RegexBuilderObject> children;
  // optional field for external identification purpose.
  int label = 0;
};

}  // namespace aparse


#endif  // APARSE_SRC_REGEX_BUILDER_HPP_
