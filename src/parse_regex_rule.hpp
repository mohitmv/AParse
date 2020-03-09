// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_PARSE_REGEX_RULE_HPP_
#define _APARSE_PARSE_REGEX_RULE_HPP_

#include "src/parse_char_regex.hpp"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>

#include "aparse/lexer.hpp"
#include "aparse/parser.hpp"

namespace aparse {


struct ParsedGrammarRule {
  enum Type {EPSILON, LITERAL, NON_TERMINAL, STRING, UNION, CONCAT, KPLUS,
             KSTAR, RULE, ALL_EXCEPT};
  ParsedGrammarRule() = default;
  explicit ParsedGrammarRule(Type type): type(type) {}
  explicit ParsedGrammarRule(const string& value): type(LITERAL),
                                          value(value) {}
  explicit ParsedGrammarRule(string&& value): type(LITERAL),
                                     value(value) {}
  ParsedGrammarRule(Type type, const vector<ParsedGrammarRule>& children)
    : type(type),
      children(children) {}
  ParsedGrammarRule(Type type, vector<ParsedGrammarRule>&& children)
    : type(type),
      children(std::move(children)) {}
  Type type;
  string value;
  vector<ParsedGrammarRule> children;
  string DebugString() const;
  ParsedGrammarRule& SetValue(const string& value) {
    this->value = value;
    return *this;
  }
  bool operator==(const ParsedGrammarRule& other) const;
};


struct ParseRegexRule {
  static ParsedGrammarRule Parse(const string& input);
  static bool Parse(const string& input,
                    ParsedGrammarRule* output,
                    Error* error);
};

namespace helpers {
AParseGrammar StringRulesToAParseGrammar(
    const vector<string>& rule_strings,
    const unordered_map<string, Alphabet>& string_to_alphabet_map,
    const vector<pair<string, string>>& branching_alphabets,
    const string& main_non_terminal);

}  // namespace helpers
}  // namespace aparse

#endif  // _APARSE_PARSE_REGEX_RULE_HPP_
