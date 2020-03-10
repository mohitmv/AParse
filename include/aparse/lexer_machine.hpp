// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_LEXER_MACHINE_HPP_
#define APARSE_LEXER_MACHINE_HPP_

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>

#include "aparse/utils/very_common_headers.hpp"

namespace aparse {

/** Underlying machine used by lexer. Currently we are using deterministic
 *  finite state automata (DFA). */
struct LexerMachine {
  struct NFA {
    struct NFAState {
      std::unordered_map<Alphabet, std::unordered_set<int>> edges;
      int label;  // optional field for external identification purpose.
    };
    std::vector<NFAState> states;
    int start_state;
    std::unordered_set<int> final_states;
    void DebugString();
  };

  struct DFA {
    struct DFAState {
      std::unordered_map<Alphabet, int> edges;
      int label;  // optional field for external identification purpose.
    };
    std::vector<DFAState> states;
    int start_state;
    std::unordered_set<int> final_states;
    std::string DebugString() const;
  };
  DFA dfa;
  std::unordered_map<int, int> section_index;
  bool initialized = false;
  std::string DebugString() const {
    return dfa.DebugString();
  }
};

}  // namespace aparse

#endif  // APARSE_LEXER_MACHINE_HPP_
