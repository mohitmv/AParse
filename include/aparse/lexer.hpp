// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_LEXER_HPP_
#define APARSE_LEXER_HPP_

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>
#include <functional>

#include "quick/stl_utils.hpp"

#include "aparse/common_headers.hpp"
#include "aparse/error.hpp"
#include "aparse/lexer_machine.hpp"
#include "aparse/utils/any.hpp"

namespace aparse {

struct LexerScopeBase {
  struct LexingConstructs {
    pair<int, int> range;
    uint32_t line_number = 1;
    uint32_t column_number = 0;
    int next_section;
  };
  pair<int, int> Range() const {
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

class Lexer;

template<typename LexerScope = LexerScopeBase>
class LexerInstance {
 public:
  LexerInstance() = default;
  LexerInstance(const Lexer& lexer, LexerScope* scope);

  void Init(const Lexer& lexer, LexerScope* scope);

  void FeedOrDie(const string& input) {
    for (auto c : input) {
      FeedOrDie(static_cast<unsigned char>(c));
    }
  }

  bool Feed(const string& input) {
    for (auto c : input) {
      if (not Feed(static_cast<unsigned char>(c))) {
        return false;
      }
    }
    return true;
  }

  bool Feed(const string& input, Error* error) {
    for (auto c : input) {
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
      scope->__lexing_constructs.range = make_pair(token_start, token_end);
      InvokeRuleAction(state->label);
      current_state = section_to_start_state_mapping->at(
                                      scope->__lexing_constructs.next_section);
      state = &machine->dfa.states[current_state];
      for (auto b : buffer) {
        if (b == static_cast<unsigned char>('\n')) {
          scope->__lexing_constructs.line_number++;
          scope->__lexing_constructs.column_number = 0;
        } else {
          scope->__lexing_constructs.column_number++;
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
      scope->__lexing_constructs.range = make_pair(token_start, token_end);
      InvokeRuleAction(machine->dfa.states[current_state].label);
      return true;
    }
    return false;
  }

  void Reset() {
    scope->__lexing_constructs.next_section = main_section;
    current_state = machine->dfa.start_state;
    token_start = 0;
    token_end = 0;
  }

 private:
  void InvokeRuleAction(int label) {
    using ActionType = std::function<void(LexerScope*)>;
    auto& action = pattern_actions->at(label);
    if (action.has_value()) {
      if (action.can_cast_to<ActionType>()) {
        action.cast_to<ActionType>()(scope);
      } else {
        throw Error(Error::INVALID_LEXER_RUN_ACTION_TYPE);
      }
    }
  }


 public:
  const LexerMachine* machine;
  const unordered_map<int, int>* section_to_start_state_mapping;
  const unordered_map<int, utils::any>* pattern_actions;
  int current_state, token_start = 0, token_end = 0;
  std::vector<Alphabet> buffer;
  int main_section;
  // not owned.
  LexerScope* scope;
};


class Lexer : public quick::AbstractType {
 public:
  string DebugString() const {
    return machine.DebugString();
  }

  template<typename LexerScope>
  LexerInstance<LexerScope> CreateInstance(LexerScope* scope) const {
    return LexerInstance<LexerScope>(*this, scope);
  }

  bool Finalize();
  bool IsInitialized() const {
    return initialized;
  }

  LexerMachine machine;
  unordered_map<int, int> section_to_start_state_mapping;
  unordered_map<int, utils::any> pattern_actions;
  int main_section;
  bool initialized = false;
};


template<typename LexerScope>
LexerInstance<LexerScope>::LexerInstance(const Lexer& lexer,
                                         LexerScope* scope) {
  this->Init(lexer, scope);
}

template<typename LexerScope>
void LexerInstance<LexerScope>::Init(const Lexer& lexer, LexerScope* scope) {
  APARSE_ASSERT(lexer.IsInitialized());
  this->scope = scope;
  this->machine = &lexer.machine;
  this->section_to_start_state_mapping = &lexer.section_to_start_state_mapping;
  this->pattern_actions = &lexer.pattern_actions;
  this->main_section = lexer.main_section;
  this->Reset();
}

}  // namespace aparse

#endif  // APARSE_LEXER_HPP_
