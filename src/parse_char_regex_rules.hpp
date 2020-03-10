// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_PARSE_CHAR_REGEX_RULES_HPP_
#define APARSE_PARSE_CHAR_REGEX_RULES_HPP_

#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

#include <quick/stl_utils.hpp>

#include "aparse/utils/any.hpp"
#include "aparse/parser.hpp"
#include "aparse/regex.hpp"
#include "aparse/aparse_grammar.hpp"

namespace aparse {

struct String2IntMapping {
  int counter;
  explicit String2IntMapping(int count_start_offset = 0)
    : counter(count_start_offset) {}
  unordered_map<string, int> s2int_map;
  int operator()(const string& s) {
    if (not qk::ContainsKey(s2int_map, s)) {
      s2int_map[s] = counter++;
    }
    return s2int_map.at(s);
  }
  void Reset(int count_start_offset = 0) {
    counter = count_start_offset;
    s2int_map.clear();
  }
};

struct CharRegexParserScope : public ParserScopeBase<Regex> {
  const string* content;
};

pair<AParseGrammar, vector<utils::any>> CharRegexParserRules();

}  // namespace aparse

#endif  //  APARSE_PARSE_CHAR_REGEX_RULES_HPP_
