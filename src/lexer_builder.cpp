// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/lexer_builder.hpp"

#include "src/internal_lexer_builder.hpp"
#include "src/parse_char_regex.hpp"

namespace aparse {

bool LexerBuilder::Build(const LexerGrammar& lexer_grammar, Lexer* lexer) {
  InternalLexerGrammar igrammar;
  Error error;
  using Rule = InternalLexerGrammar::Rule;
  for (auto& section : lexer_grammar.rules) {
    for (auto& rule : section.second) {
      Regex regex;
      if (not ParseCharRegex::Parse(rule.regex_string,
                                    &regex,
                                    &error)) {
        error.status = Error::LEXER_BUILDER_ERROR_INVALID_REGEX;
        error.string_value = rule.regex_string + "\n";
        throw error();
      }
      igrammar.rules[section.first].emplace_back(
                                            Rule(regex).Action(rule.action));
    }
  }
  igrammar.main_section = lexer_grammar.main_section;
  return InternalLexerBuilder::Build(igrammar, lexer);
}

}  // namespace aparse
