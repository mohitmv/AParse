// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/lexer_machine_builder.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <set>

#include <quick/unordered_map.hpp>
#include <quick/unordered_set.hpp>
#include <quick/debug.hpp>
#include <quick/stl_utils.hpp>

#include "src/utils.hpp"
#include "src/regex_helpers.hpp"

namespace aparse {

using DFA = LexerMachineBuilder::DFA;
using NFA = LexerMachineBuilder::NFA;

string DFA::DebugString() const {
  std::ostringstream oss;
  oss << "start_state = " << start_state << "\n";
  oss << "final_states = "  << final_states << "\n";
  for (int i = 0; i < states.size(); i++) {
    for (auto& item : states[i].edges) {
      oss << "States[" << i << "] ---(" << item.first << ")---> "
          << item.second << "\n";
    }
  }
  return oss.str();
}


NFA LexerMachineBuilder::BuildNFA(const Regex& regex) {
  struct SubNFA {
    int start_state;
    unordered_set<int> final_states;
  };
  NFA output;
  auto& states = output.states;
  // Add all the outgoing edges of state1 to state2 as well.
  auto lAddEdges = [&](const NFA::NFAState& state1, NFA::NFAState* state2) {
    for (auto& item : state1.edges) {
      for (int ts : item.second) {
        if (not ContainsKey(state2->edges[item.first], ts)) {
          state2->edges[item.first].insert(ts);
        }
      }
    }
  };
  std::function<SubNFA(const Regex&)> lRegexParsingNFAHelper = [&](
      const Regex& regex) {
    SubNFA output;
    int ni_ofst = states.size();  // node-index-offset
    switch (regex.type) {
      case Regex::EPSILON: {
        states.push_back(NFA::NFAState());
        output.start_state = ni_ofst;
        output.final_states.insert(ni_ofst);
        break;
      }
      case Regex::ATOMIC: {
        states.push_back(NFA::NFAState());
        states.push_back(NFA::NFAState());
        states[ni_ofst].edges[regex.alphabet].insert(ni_ofst+1);
        output.start_state = ni_ofst;
        output.final_states.insert(ni_ofst+1);
        break;
      }
      case Regex::UNION: {
        states.push_back(NFA::NFAState());
        output.start_state = ni_ofst;
        if (helpers::IsAllRegexChildrenAtomic(regex)) {  // Optimisation
          states.push_back(NFA::NFAState());
          for (auto& child : regex.children) {
            states[ni_ofst].edges[child.alphabet].insert(ni_ofst+1);
          }
          output.final_states.insert(ni_ofst+1);
        } else {
          bool is_start_state_also_final = false;
          for (auto& child : regex.children) {
            SubNFA snfa = lRegexParsingNFAHelper(child);
            qk::InsertToSet(snfa.final_states, &output.final_states);
            lAddEdges(states[snfa.start_state], &states[output.start_state]);
            if (not is_start_state_also_final &&
                ContainsKey(snfa.final_states, snfa.start_state)) {
              is_start_state_also_final = true;
            }
          }
          if (is_start_state_also_final) {
            output.final_states.insert(output.start_state);
          }
        }
        break;
      }
      case Regex::CONCAT: {
        unordered_set<int> previous_final_states;
        for (int i=0; i < regex.children.size(); i++) {
          SubNFA snfa = lRegexParsingNFAHelper(regex.children[i]);
          if (i == 0) {
            output.start_state = snfa.start_state;
          } else {
            for (int fs : previous_final_states) {
              lAddEdges(states[snfa.start_state], &states[fs]);
            }
          }
          if (i > 0 && ContainsKey(snfa.final_states, snfa.start_state)) {
            qk::InsertToSet(snfa.final_states, &previous_final_states);
          } else {
            previous_final_states = snfa.final_states;
          }
        }
        output.final_states = previous_final_states;
        break;
      }
      case Regex::KSTAR:
      case Regex::KPLUS: {
        states.push_back(NFA::NFAState());
        output.start_state = ni_ofst;
        SubNFA snfa = lRegexParsingNFAHelper(regex.children[0]);
        output.final_states = snfa.final_states;
        for (auto fs : snfa.final_states) {
          lAddEdges(states[snfa.start_state], &states[fs]);
        }
        lAddEdges(states[snfa.start_state], &states[output.start_state]);
        if (regex.type == Regex::KSTAR) {
          output.final_states.insert(output.start_state);
        } else if (ContainsKey(snfa.final_states, snfa.start_state)) {
          output.final_states.insert(output.start_state);
        }
        break;
      }
      default:
        assert(false);
    }
    if (regex.label > 0) {
      for (int fs : output.final_states) {
        states[fs].label = regex.label;
      }
    }
    return output;
  };
  SubNFA sub_nfa = lRegexParsingNFAHelper(regex);
  output.start_state = sub_nfa.start_state;
  output.final_states = sub_nfa.final_states;
  return output;
}

void LexerMachineBuilder::ReduceNFA(NFA& nfa) {
  vector<bool> visited(nfa.states.size(), false);
  std::function<void(int)> lDFS = [&](int u) -> void {
    visited[u] = true;
    for (auto& item : nfa.states[u].edges) {
      for (int ts : item.second) {
        if (!visited[ts]) {
          lDFS(ts);
        }
      }
    }
  };
  lDFS(nfa.start_state);
  vector<int> trap_states;
  // Make sure there is only 1 traped final state.
  for (int fs : nfa.final_states) {
    if (visited[fs] && nfa.states[fs].edges.size() == 0) {
      if (trap_states.size() > 0) {
        visited[fs] = false;
      }
      trap_states.push_back(fs);
    }
  }
  vector<NFA::NFAState> new_states;
  vector<int> new_position(nfa.states.size());
  for (int i=0; i < nfa.states.size(); i++) {
    if (visited[i]) {
      new_position[i] = new_states.size();
      new_states.push_back(nfa.states[i]);
    }
  }
  for (int i = 1; i < trap_states.size(); i++) {
    new_position[trap_states[i]] = new_position[trap_states[0]];
  }
  nfa.start_state = new_position[nfa.start_state];
  unordered_set<int> new_final_states;
  for (int fs : nfa.final_states) {
    if (visited[fs]) {
      new_final_states.insert(new_position[fs]);
    }
  }
  for (auto& state : new_states) {
    unordered_map<Alphabet, unordered_set<int>> new_edges;
    for (auto &item : state.edges) {
      for (int ts : item.second) {
        new_edges[item.first].insert(new_position[ts]);
      }
    }
    state.edges = new_edges;
  }
  nfa.states = new_states;
  nfa.final_states = new_final_states;
}

void LexerMachineBuilder::BuildDFA(const NFA& nfa, DFA* output_dfa) {
  DFA& dfa = *output_dfa;
  dfa.start_state = 0;
  qk::unordered_map<set<int>, int> dfa_index_map;
  qk::unordered_set<set<int>> cur_states;
  auto lAddDfaState = [&](const set<int> state) {
    dfa_index_map[state] = dfa.states.size();
    dfa.states.resize(dfa.states.size()+1);
    cur_states.insert(state);
  };
  lAddDfaState(set<int>({nfa.start_state}));
  while (!cur_states.empty()) {
    auto &top = *cur_states.begin();
    unordered_map<Alphabet, set<int>> next_states;
    for (int nfa_state : top) {
      for (auto& item : nfa.states[nfa_state].edges) {
        for (auto& item2 : item.second) {
          next_states[item.first].insert(item2);
        }
      }
    }
    uint dfa_index_of_top = dfa_index_map[top];
    for (auto& kv : next_states) {
      if (not qk::ContainsKey(dfa_index_map, kv.second)) {
        lAddDfaState(kv.second);
      }
      dfa.states[dfa_index_of_top].edges[kv.first] = dfa_index_map[kv.second];
    }
    for (int nfa_state : top) {
      if (ContainsKey(nfa.final_states, nfa_state)) {
        dfa.states[dfa_index_of_top].label = nfa.states[nfa_state].label;
        dfa.final_states.insert(dfa_index_of_top);
        break;
      }
    }
    cur_states.erase(top);
  }
}


// static
void LexerMachineBuilder::MergeDFA(
    const unordered_map<int, DFA>& dfa_map,
    int main_dfa_index,
    DFA* output_dfa,
    unordered_map<int, int>* start_states_mapping) {
  int offset = 0;
  for (auto& item : dfa_map) {
    auto& dfa = item.second;
    if (item.first == main_dfa_index) {
      output_dfa->start_state = offset + dfa.start_state;
    }
    qk::InsertToVector(dfa.states, &output_dfa->states);
    for (auto fs : dfa.final_states) {
      output_dfa->final_states.insert(offset + fs);
    }
    for (int i = offset; i < offset + dfa.states.size(); i++) {
      for (auto& item : output_dfa->states[i].edges) {
        item.second += offset;
      }
    }
    (*start_states_mapping)[item.first] = offset + dfa.start_state;
    offset += dfa.states.size();
  }
}

}  // namespace aparse
