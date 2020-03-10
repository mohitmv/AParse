// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_SRC_INTERNAL_LEXER_BUILDER_HPP_
#define APARSE_SRC_INTERNAL_LEXER_BUILDER_HPP_

#include <tuple>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include <quick/type_traits/function_type.hpp>

#include "aparse/regex.hpp"
#include "aparse/utils/any.hpp"
#include "aparse/lexer.hpp"

namespace aparse {

class InternalLexerGrammar {
 public:
  struct Rule {
    inline explicit Rule(Regex regex): regex(regex) {}

    template<typename T>
    std::enable_if_t<std::is_same<T, utils::any>::value, Rule&>
    Action(const T& action) {
      this->action = action;
      return *this;
    }

    template<typename T>
    std::enable_if_t<!std::is_same<T, utils::any>::value, Rule&>
    Action(const T& action) {
      this->action = quick::function_type<T>(action);
      return *this;
    }

    Regex regex;
    utils::any action;
  };
  std::unordered_map<int, std::vector<Rule>> rules;
  int main_section = 0;
};


class InternalLexerBuilder {
 public:
  static bool Build(const InternalLexerGrammar& lexer_grammar, Lexer* lexer);
};

}  // namespace aparse

#endif  // APARSE_SRC_INTERNAL_LEXER_BUILDER_HPP_
