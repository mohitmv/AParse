// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_LEXER_MACHINE_BUILDER_HPP_
#define _APARSE_LEXER_MACHINE_BUILDER_HPP_

#include "aparse/regex.hpp"
#include "aparse/common_headers.hpp"
#include "aparse/lexer_machine.hpp"


namespace aparse {


class LexerMachineBuilder {
public:
  static LexerMachine::NFA BuildNFA(const Regex& regex);
  void ReduceNFA(LexerMachine::NFA& nfa);
  using DFA = LexerMachine::DFA;
  using NFA = LexerMachine::NFA;
  static DFA BuildDFA(const NFA& nfa);
  static void MergeDFA(const unordered_map<int, DFA>& dfa_map,
                       int main_dfa,
                       DFA* output_dfa,
                       unordered_map<int, int>* start_states_mapping);
};



}  // namespace aparse




#endif  // _APARSE_LEXER_MACHINE_BUILDER_HPP_

