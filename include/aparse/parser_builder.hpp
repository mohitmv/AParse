// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_PARSER_BUILDER_HPP_
#define _APARSE_PARSER_BUILDER_HPP_

#include <tuple>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

#include <quick/type_traits/function_type.hpp>

#include "aparse/utils/very_common_headers.hpp"
#include "aparse/parser.hpp"
#include "src/internal_parser_builder.hpp"

namespace aparse {

class ParserGrammar {
 public:
  struct Rule {
    explicit Rule(const std::string& rule): rule_string(rule) {}
    template<typename T>
    Rule& Action(const T& action) {
      this->action = quick::function_type<T>(action);
      return *this;
    }
    std::string rule_string;
    utils::any action;
  };

  std::vector<Rule> rules;
  string main_non_terminal;
  std::vector<std::pair<std::string, std::string>> branching_alphabets;
  std::unordered_map<string, Alphabet> string_to_alphabet_map;
};


class ParserBuilder {
 public:
  static bool Import(const std::string& serialized_parser,
              const ParserGrammar& parser_grammar,
              Parser* parser);

  static void Export(const Parser& parser,
              const ParserGrammar& parser_grammar,
              std::string* serialized_parser);

  static std::string Export(const Parser& parser,
                     const ParserGrammar& parser_grammar);

  static void Build(const ParserGrammar& parser_grammar, Parser* parser);
};


}  // namespace aparse

#endif  // _APARSE_PARSER_BUILDER_HPP_
