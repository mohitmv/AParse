// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_LEXER_BUILDER_HPP_
#define APARSE_LEXER_BUILDER_HPP_

#include <tuple>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

#include <quick/type_traits/function_type.hpp>

#include "aparse/utils/very_common_headers.hpp"
#include "aparse/lexer.hpp"
#include "aparse/utils/any.hpp"

namespace aparse {

/** LexerGrammar is used for defining the lexer grammar.
 *  A Lexer Rule is a pair of Regex and the Action that needs to be done when
 *  that regex is matched in input string.
 *  Learn more at : https://aparse.readthedocs.io
 *  Learn more at : 'src/lexer_builder_integration_test.cpp' */
class LexerGrammar {
 public:
  struct Rule {
    explicit Rule(const string& regex_string) : regex_string(regex_string) {
    }
    template<typename T>
    Rule& Action(const T& action) {
      this->action = quick::function_type<T>(action);
      return *this;
    }
    string regex_string;
    utils::any action;
  };
  unordered_map<int, vector<Rule>> rules;
  int main_section = 0;
  bool Finalize();
};

class LexerBuilder {
 public:
  static bool Build(const LexerGrammar& lexer_grammar, Lexer* lexer);
};

}  // namespace aparse

#endif  // APARSE_LEXER_BUILDER_HPP_
