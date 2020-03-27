// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/aparse_machine.hpp"

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <sstream>


namespace aparse {

string AParseMachine::GetBranchSymbolTypeString(
    AParseMachine::BranchSymbolType t) {
  switch (t) {
    case START_BRANCH: return "START_BRANCH";
    case END_BRANCH: return "END_BRANCH";
    default: assert(false);
  }
  return "";
}

string AParseMachine::ShortDebugString() const {
  std::ostringstream oss;
  oss << "NFA: {\n";
  oss << " number of states = " << nfa.states.size() << std::endl;
  oss << " num final states = " << nfa.final_states.size() << std::endl;
  oss << "}\nDFA: {" << std::endl;
  oss << " number of states = " << dfa.states.size() << std::endl;
  oss << " num final states = " << dfa.final_states.size() << std::endl;
  oss << "}" << std::endl;
  return oss.str();
}

using StackOperation = AParseMachine::StackOperation;

string StackOperation::GetTypeString(StackOperation::OperationType type) {
  switch (type) {
    case StackOperation::NOOP: return "NOP";
    case StackOperation::PUSH: return "PUSH";
    case StackOperation::POP: return "POP";
    default: assert(false);
  }
  return "";
}

string StackOperation::GetTypeString() const {
  return GetTypeString(type);
}

string AParseMachine::DebugString() const {
  std::ostringstream oss;
  auto lPrintStackOps = [&](const StackOperation& sop) {
    switch (sop.type) {
      case StackOperation::NOOP: return;
      case StackOperation::PUSH: oss << "Push(" << sop.value << ")"; return;
      case StackOperation::POP: oss << "Pop"; return;
      default: assert(false);
    }
  };
  auto lPrintParsingStream = [&](const ParsingStream& ps) {
    for (auto& item : ps) {
      oss << (item.first == START_BRANCH ? '(': ')') << item.second << ",";
    }
  };
  auto lPrintVector = [&](const vector<int>& v) {
    oss << "[";
    for (auto e : v) {
      oss << e << ",";
    }
    oss << "]";
  };
  oss << "\n\n----- NFA (" << nfa.states.size() << " states, "
      << nfa.final_states.size() << " final states, start_state_index = "
      << nfa.start_state << "---\n\n";
  for (int i=0; i< nfa.states.size(); i++) {
    auto& state = nfa.states[i];
    bool is_final = ContainsKey(nfa.final_states, i);
    oss << "State " << i << " "
        << (is_final ? "[F] ": " ")
        << ": \n";
    for (auto& item : state.edges) {
      for (auto& item2 : item.second) {
        oss << "  -- "<< item.first << " --> " << item2.first << "{ ";
        lPrintStackOps(item2.second.stack_operation);
        oss << ", ";
        lPrintParsingStream(item2.second.parsing_stream);
        oss << "}\n";
      }
    }
    for (auto& item : state.special_edges) {
      for (auto& item2 : item.second) {
        oss << "  -- ("<< item.first.first << "," << item.first.second
            << ") --> " << item2.first << "{ ";
        lPrintStackOps(item2.second.stack_operation);
        oss << ", ";
        lPrintParsingStream(item2.second.parsing_stream);
        oss << "}\n";
      }
    }
    if (is_final || true) {
      oss << "State parsing_stream = ";
      lPrintParsingStream(state.parsing_stream);
      oss << "\n";
    }
  }
  oss << "\n\n----- DFA (" << dfa.states.size() << " states, "
      << dfa.final_states.size() << " final states, start_state_index = "
      << dfa.start_state << "---\n\n";
  for (int i=0; i< dfa.states.size(); i++) {
    auto& state = dfa.states[i];
    bool is_final = ContainsKey(dfa.final_states, i);
    oss << "State " << i << " "
        << (is_final ? "[F] ": " ")
        << ": \n";
    for (auto& item : state.edges) {
      oss << "  --"<< item.first << "--> " << item.second.first << " { ";
      lPrintStackOps(item.second.second);
      oss << "}\n";
    }
    for (auto& item : state.special_edges) {
      oss << "  -- ("<< item.first.first << "," << item.first.second
          << ") --> " << item.second.first << " { ";
      lPrintStackOps(item.second.second);
      oss << "}\n";
    }
    oss << "Corresponding NFA States = ";
    lPrintVector(state.nfa_states);
    oss << "\n";
  }
  oss << "\n\n--- Map1 : \n";
  for (auto& item : back_tracking.map1) {
    oss << "(" << std::get<0>(item.first) << "," << std::get<1>(item.first) << ","
        << std::get<2>(item.first) << ") : " << item.second << "\n";
  }
  oss << "-----\n\n";
  return oss.str();
}


bool AParseMachine::StackOperation::operator==(
    const AParseMachine::StackOperation& o) const {
  return (type == o.type && value == o.value);
}

void AParseMachine::StackOperation::Serialize(qk::OByteStream& bs) const {
  bs << type;
  if (type == PUSH) {
    bs << value;
  }
}

void AParseMachine::StackOperation::Deserialize(qk::IByteStream& bs) {
  bs >> type;
  if (type == PUSH) {
    bs >> value;
  }
}

bool AParseMachine::NFA::EdgeLabel::operator==(
    const AParseMachine::NFA::EdgeLabel& o) const {
  return (parsing_stream == o.parsing_stream &&
          stack_operation == o.stack_operation);
}


void AParseMachine::NFA::EdgeLabel::Serialize(qk::OByteStream& bs) const {
  bs << parsing_stream << stack_operation;
}

void AParseMachine::NFA::EdgeLabel::Deserialize(qk::IByteStream& bs) {
  bs >> parsing_stream >> stack_operation;
}

bool AParseMachine::NFA::NFAState::operator==(
    const AParseMachine::NFA::NFAState& o) const {
  return (edges == o.edges &&
          special_edges == o.special_edges &&
          parsing_stream == o.parsing_stream);
}

void AParseMachine::NFA::NFAState::Serialize(qk::OByteStream& bs) const {
  bs << edges << special_edges << parsing_stream << label;
}

void AParseMachine::NFA::NFAState::Deserialize(qk::IByteStream& bs) {
  bs >> edges >> special_edges >> parsing_stream >> label;
}

bool AParseMachine::NFA::operator==(const AParseMachine::NFA& o) const {
  return (states == o.states &&
          special_parsing_stream == o.special_parsing_stream &&
          start_state == o.start_state &&
          final_states == o.final_states);
}

void AParseMachine::NFA::Serialize(qk::OByteStream& bs) const {
  bs << states << special_parsing_stream << start_state << final_states;
}

void AParseMachine::NFA::Deserialize(qk::IByteStream& bs) {
  bs >> states >> special_parsing_stream >> start_state >> final_states;
}

bool AParseMachine::DFA::DFAState::operator==(
      const AParseMachine::DFA::DFAState& o) const {
  return (edges == o.edges &&
          special_edges == o.special_edges &&
          nfa_states == o.nfa_states);
}

void AParseMachine::DFA::DFAState::Serialize(qk::OByteStream& bs) const {
  bs << edges << special_edges << nfa_states << label;
}

void AParseMachine::DFA::DFAState::Deserialize(qk::IByteStream& bs) {
  bs >> edges >> special_edges >> nfa_states >> label;
}

bool AParseMachine::DFA::operator==(const AParseMachine::DFA& o) const {
  return (states == o.states &&
          start_state == o.start_state &&
          final_states == o.final_states &&
          nfa_final_state_map == o.nfa_final_state_map);
}

void AParseMachine::DFA::Serialize(qk::OByteStream& bs) const {
  bs << states << start_state << final_states << nfa_final_state_map;
}

void AParseMachine::DFA::Deserialize(qk::IByteStream& bs) {
  bs >> states >> start_state >> final_states >> nfa_final_state_map;
}

bool AParseMachine::BackTracking::operator==(const AParseMachine::BackTracking& o) const {
  return (map1 == o.map1 &&
          map2 == o.map2 &&
          jump_parsing_stream == o.jump_parsing_stream);
}

void AParseMachine::BackTracking::Serialize(qk::OByteStream& bs) const {
  bs << map1 << map2 << jump_parsing_stream;
}

void AParseMachine::BackTracking::Deserialize(qk::IByteStream& bs) {
  bs >> map1 >> map2 >> jump_parsing_stream;
}

bool AParseMachine::operator==(const AParseMachine& o) const {
  return (nfa == o.nfa &&
          dfa == o.dfa &&
          back_tracking == o.back_tracking);
}

void AParseMachine::Serialize(qk::OByteStream& bs) const {
  bs << nfa << dfa << back_tracking;
}

void AParseMachine::Deserialize(qk::IByteStream& bs) {
  bs >> nfa >> dfa >> back_tracking;
}

// ToDo(Mohit): So many copies of serialized_machine are created in
// import/export. Optimise it.
// Format Version - 2
std::string AParseMachine::Export() const {
  qk::OByteStream obs;
  uint32_t version = 2;
  obs << version << *this;
  return obs.str();
}

// Format Version - 2
bool AParseMachine::Import(const std::string& serialized_machine) {
  qk::IByteStream ibs;
  ibs.str(serialized_machine);
  uint32_t expected_version = 2, current_version;
  ibs >> current_version;
  if (current_version != expected_version) {
    return false;
  }
  ibs >> *this;
  return true;
}



}  // namespace aparse

