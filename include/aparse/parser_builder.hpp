// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_PARSER_BUILDER_HPP_
#define _APARSE_PARSER_BUILDER_HPP_

#include <tuple>

#include <quick/type_traits/function_type.hpp>

#include "aparse/common_headers.hpp"
#include "aparse/error.hpp"
#include "aparse/parser.hpp"
#include "aparse/internal_parser_builder.hpp"

namespace aparse {

class ParserGrammar {
 public:
  struct Rule {
    Rule (const string& rule): rule_string(rule) {}
    template<typename T>
    Rule& Action(const T& action) {
      this->action = quick::function_type<T>(action);
      return *this;
    }
    string rule_string;
    utils::any action;
  };
  vector<Rule> rules;
  string main_non_terminal;
  vector<pair<string, string>> branching_alphabets;
  unordered_map<string, Alphabet> string_to_alphabet_map;
};


class ParserBuilder {
public:
  bool Import(const string& serialized_parser,
              const ParserGrammar& parser_grammar,
              Parser* parser);

  void Build(const ParserGrammar& parser_grammar, Parser* parser);
};


}  // namespace aparse

#endif  // _APARSE_PARSER_BUILDER_HPP_
