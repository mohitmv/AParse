// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_INTERNAL_LEXER_HPP_
#define _APARSE_INTERNAL_LEXER_HPP_

#include <functional>

#include "quick/utility.hpp"
#include "quick/stl_utils.hpp"
#include "aparse/common_headers.hpp"
#include "aparse/regex.hpp"
#include "aparse/error.hpp"
#include "aparse/lexer_machine.hpp"
#include "aparse/lexer_machine_builder.hpp"

namespace aparse {

struct InternalLexerScopeBase {
  struct LexingConstructs {
    pair<int, int> range;
    uint32_t line_number = 1;
    uint32_t column_number = 0;
    int next_section;
  };
  pair<int,int> Range() const {
    return __lexing_constructs.range;
  }
  uint32_t LineNumber() const {
    return __lexing_constructs.line_number;
  }
  uint32_t ColumnNumber() const {
    return __lexing_constructs.column_number;
  }
  void JumpTo(int next_section) {
    __lexing_constructs.next_section = next_section;
  }
  LexingConstructs __lexing_constructs;
};

template<typename LexerScopeT = InternalLexerScopeBase>
class InternalLexerRules {
 public:
  using LexerScope = LexerScopeT;
  struct Rule {
    Regex regex;
    string regex_string;
    std::function<void(LexerScope*)> action;
    bool has_action = false;
    Rule(const string& regex_string): regex_string(regex_string) {
      action = [](LexerScope*){};  // Remove this and handle has_action flag.
    }
    Rule(Regex regex): regex(regex) {
      action = [](LexerScope*){};  // Remove this and handle has_action flag.
    }
    Rule& Action(std::function<void(LexerScope*)> action) {
      this->action = action;
      this->has_action = true;
      return *this;
    }
  };
  unordered_map<int, vector<Rule>> rules;
  int main_section = 0;
};

template<typename LexerScope = InternalLexerScopeBase>
class InternalLexer;


template<typename LexerScope = InternalLexerScopeBase>
class InternalLexerInstance {
 public:
  InternalLexerInstance() = default;
  InternalLexerInstance(const InternalLexer<LexerScope>& lexer);

  void Init(const InternalLexer<LexerScope>& lexer);

  void FeedOrDie(const string& input) {
    for (auto c: input) {
      FeedOrDie(static_cast<unsigned char>(c));
    }
  }

  bool Feed(const string& input) {
    for (auto c: input) {
      if (not Feed(static_cast<unsigned char>(c))) {
        return false;
      }
    }
    return true;
  }

  bool Feed(const string& input, Error* error) {
    for (auto c: input) {
      if (not Feed(static_cast<unsigned char>(c), error)) {
        return false;
      }
    }
    return true;
  }

  void FeedOrDie(Alphabet a) {
    if (not Feed(a)) {
      throw Error(Error::LEXER_ERROR_INVALID_TOKENS)
                  .Position(make_pair(token_start, token_end+1))();
    }
  }

  bool Feed(Alphabet a, Error* error) {
    if (not Feed(a)) {
      *error = Error(Error::LEXER_ERROR_INVALID_TOKENS)
                      .Position(make_pair(token_start, token_end+1))();
      return false;
    }
    return true;
  }

  bool Feed(Alphabet a) {
    const auto* state = &machine->dfa.states[current_state];
    if (not qk::ContainsKey(state->edges, a) &&
        qk::ContainsKey(machine->dfa.final_states, current_state)) {
      scope.__lexing_constructs.range = make_pair(token_start, token_end);
      pattern_actions->at(state->label)(&scope);
      current_state = section_to_start_state_mapping->at(
                                      scope.__lexing_constructs.next_section);
      state = &machine->dfa.states[current_state];
      for (auto b: buffer) {
        if (b == static_cast<unsigned char>('\n')) {
          scope.__lexing_constructs.line_number++;
          scope.__lexing_constructs.column_number = 0;
        } else {
          scope.__lexing_constructs.column_number++;
        }
      }
      token_start = token_end;
      buffer.clear();
    }
    if (qk::ContainsKey(state->edges, a)) {
      current_state = state->edges.at(a);
    } else {
      return false;
    }
    buffer.push_back(a);
    token_end++;
    return true;
  }

  void EndOrDie() {
    if (not End()) {
      throw Error(Error::LEXER_ERROR_INCOMPLETE_TOKENS)
                  .Position(make_pair(token_start, token_end+1))();
    }
  }

  bool End(Error* error) {
    if (not End()) {
      *error = Error(Error::LEXER_ERROR_INCOMPLETE_TOKENS)
                  .Position(make_pair(token_start, token_end+1))();
      return false;
    }
    return true;
  }

  bool End() {
    if (qk::ContainsKey(machine->dfa.final_states, current_state)) {
      scope.__lexing_constructs.range = make_pair(token_start, token_end);
      pattern_actions->at(machine->dfa.states[current_state].label)(&scope);
      return true;
    }
    return false;
  }

  void Reset() {
    scope.__lexing_constructs.next_section = main_section;
    current_state = machine->dfa.start_state;
    token_start = 0;
    token_end = 0;
  }

  LexerScope& Scope() {
    return scope;
  }
  const LexerMachine* machine;
  const unordered_map<int, int>* section_to_start_state_mapping;
  const unordered_map<int, std::function<void(LexerScope*)>>* pattern_actions;
  int current_state, token_start = 0, token_end = 0;
  std::vector<Alphabet> buffer;
  int main_section;
  LexerScope scope;
};


template<typename LexerScope>
class InternalLexer : public quick::AbstractType {
 public:
  using Instance = InternalLexerInstance<LexerScope>;
  using LexerRules = InternalLexerRules<LexerScope>;
  using Rule = typename LexerRules::Rule;
  ~InternalLexer() = default;
  bool Build(const LexerRules& lexer_rules);
  string DebugString() const {
    return machine.DebugString();
  }
  Instance CreateInstance() const {
    return InternalLexerInstance<LexerScope>(*this);
  }
  LexerMachine machine;
  unordered_map<int, int> section_to_start_state_mapping;
  unordered_map<int, std::function<void(LexerScope*)>> pattern_actions;
  InternalLexerInstance<LexerScope> instance;
  int main_section;
};


template<typename LexerScope>
InternalLexerInstance<LexerScope>::InternalLexerInstance(
    const InternalLexer<LexerScope>& lexer) {
  this->Init(lexer);
}

template<typename LexerScope>
void InternalLexerInstance<LexerScope>::Init(
    const InternalLexer<LexerScope>& lexer) {
  _APARSE_DEBUG_ASSERT(lexer.machine.initialized);
  this->machine = &lexer.machine;
  this->section_to_start_state_mapping = &lexer.section_to_start_state_mapping;
  this->pattern_actions = &lexer.pattern_actions;
  this->main_section = lexer.main_section;
  this->Reset();
}

template<typename LexerRules, typename Lexer>
bool BuildCoreLexer(const LexerRules& input_lexer_rules, Lexer* output) {
  auto lexer_rules = input_lexer_rules;
  if (lexer_rules.rules.size() == 0) {
    throw Error(Error::LEXER_BUILDER_ERROR_MUST_HAVE_NON_ZERO_RULES)();
  }
  int index = 1;
  unordered_map<int, LexerMachine::DFA> dfa_map;
  vector<int> section_id_list;
  for (auto& section: lexer_rules.rules) {
    Regex regex(Regex::UNION);
    for (auto& rule: section.second) {
      rule.regex.label = index++;
      regex.children.push_back(rule.regex);
      output->pattern_actions[rule.regex.label] = rule.action;
    }
    auto nfa = LexerMachineBuilder::BuildNFA(regex);
    dfa_map[section.first] = std::move(LexerMachineBuilder::BuildDFA(nfa));
  }
  output->main_section = lexer_rules.main_section;
  LexerMachineBuilder::MergeDFA(
    dfa_map,
    lexer_rules.main_section,
    &output->machine.dfa,
    &output->section_to_start_state_mapping);
  output->machine.initialized = true;
  return true;
}


template<typename LexerScope>
bool InternalLexer<LexerScope>::Build(
    const InternalLexerRules<LexerScope>& lexer_rules) {
  return BuildCoreLexer(lexer_rules, this);
}


}  // namespace aparse


#endif  // _APARSE_INTERNAL_LEXER_HPP_
