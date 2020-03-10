// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_LEXER_MACHINE_BUILDER_HPP_
#define APARSE_LEXER_MACHINE_BUILDER_HPP_

#include <unordered_map>

#include "aparse/regex.hpp"
#include "aparse/utils/very_common_headers.hpp"
#include "aparse/lexer_machine.hpp"

namespace aparse {
class LexerMachineBuilder {
 public:
  static LexerMachine::NFA BuildNFA(const Regex& regex);
  void ReduceNFA(LexerMachine::NFA& nfa);  // NOLINT
  using DFA = LexerMachine::DFA;
  using NFA = LexerMachine::NFA;
  static void BuildDFA(const NFA& nfa, DFA* dfa);
  static void MergeDFA(const std::unordered_map<int, DFA>& dfa_map,
                       int main_dfa,
                       DFA* output_dfa,
                       std::unordered_map<int, int>* start_states_mapping);
};
}  // namespace aparse

#endif  // APARSE_LEXER_MACHINE_BUILDER_HPP_
