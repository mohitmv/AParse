// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

/** This utility is used for converting a grammar rule string to Regex and
 *  non-terminal pair. Grammar rules are easy to express in string form hence
 *  we allow the AParse's clients to express their rules in C++ string. If a
 *  string is not valid grammar rule then this utility returns the detailed
 *  error.
 *  Learn more at `src/parse_regex_rule_test.cpp` */

#ifndef APARSE_PARSE_REGEX_RULE_HPP_
#define APARSE_PARSE_REGEX_RULE_HPP_

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

#endif  // APARSE_PARSE_REGEX_RULE_HPP_
