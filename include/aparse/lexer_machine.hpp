// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_LEXER_MACHINE_HPP_
#define _APARSE_LEXER_MACHINE_HPP_



#include "aparse/common_headers.hpp"

namespace aparse {

struct LexerMachine {
  struct NFA {
    struct NFAState {
      unordered_map<Alphabet, unordered_set<int>> edges;
      int label; // optional field for external identification purpose.
    };
    vector<NFAState> states;
    int start_state;
    unordered_set<int> final_states;
    void DebugString();
  };

  struct DFA {
    struct DFAState {
      unordered_map<Alphabet, int> edges;
      int label; // optional field for external identification purpose.
    };
    vector<DFAState> states;
    int start_state;
    unordered_set<int> final_states;
    string DebugString() const;
  };
  DFA dfa;
  unordered_map<int, int> section_index;
  bool initialized = false;
  string DebugString() const {
    return dfa.DebugString();
  }
};




}  // namespace aparse



#endif  // _APARSE_LEXER_MACHINE_HPP_
