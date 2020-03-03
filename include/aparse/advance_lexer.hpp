// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_ADVANCE_LEXER_HPP_
#define _APARSE_ADVANCE_LEXER_HPP_


#include "aparse/common_headers.hpp"
#include "aparse/regex.hpp"
#include "aparse/error.hpp"
#include "aparse/internal_lexer.hpp"

namespace aparse {
namespace helpers {

bool ParseCharRegexFunc(const string& input, Regex* output, Error* error);

}  // namespace helpers


class AdvanceLexerScopeBase: public InternalLexerScopeBase {};

template<typename LexerScope = AdvanceLexerScopeBase>
class AdvanceLexerRules: public InternalLexerRules<LexerScope> {};

template<typename LexerScope = AdvanceLexerScopeBase>
class AdvanceLexer;

template<typename LexerScope = AdvanceLexerScopeBase>
class AdvanceLexerInstance: public InternalLexerInstance<LexerScope> {
 public:
  AdvanceLexerInstance() = default;
  AdvanceLexerInstance(const AdvanceLexer<LexerScope>& lexer);
  void Init(const AdvanceLexer<LexerScope>& lexer);
};


template<typename LexerScope>
class AdvanceLexer: public InternalLexer<LexerScope> {
 public:
  using Instance = AdvanceLexerInstance<LexerScope>;
  using LexerRules = AdvanceLexerRules<LexerScope>;
  using Rule = typename LexerRules::Rule;
  Instance CreateInstance() const {
    return AdvanceLexerInstance<LexerScope>(*this);
  }
  bool Build(const LexerRules&);
  ~AdvanceLexer() = default;
};



template<typename LexerScope>
AdvanceLexerInstance<LexerScope>::AdvanceLexerInstance(
    const AdvanceLexer<LexerScope>& lexer) {
  this->Init(lexer);
}

template<typename LexerScope>
void AdvanceLexerInstance<LexerScope>::Init(
    const AdvanceLexer<LexerScope>& lexer) {
  _APARSE_DEBUG_ASSERT(lexer.machine.initialized);
  this->machine = &lexer.machine;
  this->section_to_start_state_mapping = &lexer.section_to_start_state_mapping;
  this->pattern_actions = &lexer.pattern_actions;
  this->main_section = lexer.main_section;
  this->Reset();
}

template<typename LexerScope>
bool AdvanceLexer<LexerScope>::Build(
      const AdvanceLexerRules<LexerScope>& input_lexer_rules) {
  auto lexer_rules = input_lexer_rules;
  Error error;
  for (auto& section: lexer_rules.rules) {
    for (auto& rule: section.second) {
      if (not helpers::ParseCharRegexFunc(rule.regex_string,
                                    &rule.regex,
                                    &error)) {
        error.status = Error::LEXER_BUILDER_ERROR_INVALID_REGEX;
        error.string_value = rule.regex_string + "\n";
        throw error();
      }
    }
  }
  return BuildCoreLexer(lexer_rules, this);
}



}  // namespace aparse

#endif  // _APARSE_ADVANCE_LEXER_HPP_




