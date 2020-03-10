// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/v2/aparse_machine.hpp"

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <sstream>
#include <quick/debug_stream.hpp>
#include <quick/stl_utils.hpp>

#include <quick/debug.hpp>

namespace aparse {
namespace v2 {

using NFAState = AParseMachine::NFAState;
using NFA = AParseMachine::NFA;
using SpecialOutgoingEdges = AParseMachine::SpecialOutgoingEdges;
using OutgoingEdges = AParseMachine::OutgoingEdges;
using BranchSymbolType = AParseMachine::BranchSymbolType;

qk::DebugStream& operator<<(qk::DebugStream& ds, BranchSymbolType bst) {
  ds << AParseMachine::GetBranchSymbolTypeString(bst);
  return ds;
}


bool NFAState::operator==(const NFAState& other) const {
  return (number == other.number && full_path == other.full_path);
}

bool NFAState::operator!=(const NFAState& other) const {
  return not(this->operator==(other));
}

void AParseMachine::NFAState::Serialize(qk::OByteStream& bs) const {
  bs << full_path << number;
}

void AParseMachine::NFAState::Deserialize(qk::IByteStream& bs) {
  bs >> full_path >> number;
}

bool AParseMachine::StackOperation::operator==(
    const AParseMachine::StackOperation& o) const {
  return (type == o.type && enclosed_non_terminal == o.enclosed_non_terminal);
}

void AParseMachine::StackOperation::Serialize(qk::OByteStream& bs) const {
  bs << type << enclosed_non_terminal;
}

void AParseMachine::StackOperation::Deserialize(qk::IByteStream& bs) {
  bs >> type >> enclosed_non_terminal;
}


bool AParseMachine::NFA::operator==(const AParseMachine::NFA& other) const {
  return (edges == other.edges && special_edges == other.special_edges);
}

void AParseMachine::NFA::Serialize(qk::OByteStream& bs) const {
  bs << edges << special_edges;
}

void AParseMachine::NFA::Deserialize(qk::IByteStream& bs) {
  bs >> edges >> special_edges;
}

bool AParseMachine::EnclosedSubNFA::operator==(
    const AParseMachine::EnclosedSubNFA& other) const {
  return (final_states == other.final_states and
          start_state == other.start_state);
}

void AParseMachine::EnclosedSubNFA::Serialize(qk::OByteStream& bs) const {
  bs << final_states << start_state;
}

void AParseMachine::EnclosedSubNFA::Deserialize(qk::IByteStream& bs) {
  bs >> final_states >> start_state;
}

bool AParseMachine::operator==(const AParseMachine& other) const {
  return (nfa_map == other.nfa_map and
          start_state == other.start_state and
          final_states == other.final_states and
          nfa_lookup_map == other.nfa_lookup_map and
          enclosed_subnfa_map == other.enclosed_subnfa_map);
}

void AParseMachine::Serialize(qk::OByteStream& bs) const {
  bs << nfa_map << start_state << final_states << nfa_lookup_map
     << enclosed_subnfa_map;
}

void AParseMachine::Deserialize(qk::IByteStream& bs) {
  bs >> nfa_map >> start_state >> final_states >> nfa_lookup_map
     >> enclosed_subnfa_map;
}

pair<int, int> NFAState::GetI(int i) const {
  return full_path.at(i);
}

vector<pair<int, int>> NFAState::GetFullPath() const {
  return full_path;
}

int NFAState::GetFullPathSize() const {
  return full_path.size();
}


bool NFAState::IsLocal() const {
  return (full_path.size() == 0);
}


std::size_t NFAState::GetHash() const {
  return qk::HashFunction(number, full_path);
}

std::string NFAState::DebugString() const {
  std::ostringstream oss;
  for (auto& x : full_path) {
    oss << "[" << x.first << "-" << x.second << "]";
  }
  oss << number;
  return oss.str();
}

NFAState NFAState::AddPrefix(int non_terminal, int nfa_id) const {
  NFAState output(number);
  output.full_path.push_back(make_pair(non_terminal, nfa_id));
  qk::InsertToVector(full_path, &output.full_path);
  return output;
}

NFAState NFAState::AddPrefix(
    const vector<pair<int, int>>& specifiers) const {
  NFAState output(number);
  output.full_path = specifiers;
  qk::InsertToVector(full_path, &output.full_path);
  return output;
}

vector<std::pair<int, int>> NFAState::GetPrefix(int len) const {
  return vector<std::pair<int, int>>(full_path.begin(),
                                     std::next(full_path.begin(), len));
}

NFAState NFAState::GetSuffix(int len) const {
  NFAState output(number);
  output.full_path = vector<std::pair<int, int>>(
                          std::next(full_path.begin(), len),
                          full_path.end());
  return output;
}

NFAState NFAState::AddPrefixFromOther(const NFAState& other, int len) const {
  return AddPrefix(other.GetPrefix(len));
}


string AParseMachine::GetBranchSymbolTypeString(
    AParseMachine::BranchSymbolType t) {
  switch (t) {
    case BRANCH_START_MARKER: return "BRANCH_START_MARKER";
    case BRANCH_END_MARKER: return "BRANCH_END_MARKER";
    default: assert(false);
  }
  return "";
}

bool AParseMachine::IsFinalState(const NFAState& state) const {
  return qk::ContainsKey(final_states, state);
}

string AParseMachine::ShortDebugString() const {
  std::ostringstream oss;
  return oss.str();
}

using StackOperation = AParseMachine::StackOperation;

string StackOperation::GetTypeString(StackOperation::OperationType type) {
  switch (type) {
    case StackOperation::NOP: return "NOP";
    case StackOperation::PUSH: return "PUSH";
    case StackOperation::POP: return "POP";
    default: assert(false);
  }
  return "";
}

string StackOperation::GetTypeString() const {
  return GetTypeString(type);
}


using EnclosedSubNFA = AParseMachine::EnclosedSubNFA;
void EnclosedSubNFA::DebugStream(qk::DebugStream& ds) const {
  ds << "start_state = " << start_state << "\n"
     << "final_states = " << final_states;
}


void AParseMachine::DebugStream(qk::DebugStream& ds) const {
  ds << "nfa_map = " << nfa_map << "\n"
     << "start_state = " << start_state << "\n"
     << "final_states = " << final_states << "\n"
     << "nfa_lookup_map = " << nfa_lookup_map << "\n"
     << "enclosed_subnfa_map = " << enclosed_subnfa_map;
}


void StackOperation::DebugStream(qk::DebugStream& ds) const {
  ds << GetTypeString() << "(" << enclosed_non_terminal << ")";
}

void NFA::DebugStream(qk::DebugStream& ds) const {
  ds << "edges = " << edges << "\n"
     << "special_edges" << special_edges;
}

vector<pair<int, const OutgoingEdges*>> AParseMachine::GetOutgoingEdgesList(
      const NFAState& s) const {
  vector<pair<int, const OutgoingEdges*>> output;
  if (qk::ContainsKey(nfa_lookup_map, s)) {
    auto& nfa = nfa_map.at(nfa_lookup_map.at(s));
    if (qk::ContainsKey(nfa.edges, s)) {
      output.emplace_back(make_pair(0, &nfa.edges.at(s)));
    }
  }
  for (int i = 0; i < s.GetFullPathSize(); i++) {
    auto suffix = s.GetSuffix(i+1);
    auto& edges = nfa_map.at(s.GetI(i).first).edges;
    if (qk::ContainsKey(edges, suffix)) {
      output.emplace_back(make_pair(i+1, &edges.at(suffix)));
    }
  }
  return output;
}

// return Vector<Pair(1. number of prefixed stripped from NFAState,
//                    2. Corrosponding outgoing edges)>
vector<std::pair<int, const SpecialOutgoingEdges*>>
AParseMachine::GetSpecialOutgoingEdgesList(const NFAState& s) const {
  vector<std::pair<int, const SpecialOutgoingEdges*>> output;
  if (qk::ContainsKey(nfa_lookup_map, s)) {
    auto& nfa = nfa_map.at(nfa_lookup_map.at(s));
    if (qk::ContainsKey(nfa.special_edges, s)) {
      output.emplace_back(make_pair(0, &nfa.special_edges.at(s)));
    }
  }
  for (int i = 0; i < s.GetFullPathSize(); i++) {
    auto suffix = s.GetSuffix(i+1);
    auto& special_edges = nfa_map.at(s.GetI(i).first).special_edges;
    if (qk::ContainsKey(special_edges, suffix)) {
      output.emplace_back(make_pair(i+1, &special_edges.at(suffix)));
    }
  }
  return output;
}

void AParseMachine::GetNextStates(const NFAState& s,
                                  Alphabet a,
                                  qk::unordered_set<NFAState>* output) const {
  auto edges_list = GetOutgoingEdgesList(s);
  for (auto& item : edges_list) {
    auto& edges = *item.second;
    if (qk::ContainsKey(edges, a)) {
      auto& tmp = edges.at(a);
      for (auto& item2 : tmp) {
        output->insert(item2.first.AddPrefixFromOther(s, item.first));
      }
    }
  }
}

qk::unordered_set<NFAState> AParseMachine::GetNextStates(const NFAState& s,
                                                         Alphabet a) const {
  qk::unordered_set<NFAState> output;
  GetNextStates(s, a, &output);
  return output;
}

void AParseMachine::GetNextStackOps(const NFAState& s,
                                    Alphabet a,
                                    vector<StackOperation>* output) const {
  auto edges_list = GetSpecialOutgoingEdgesList(s);
  for (auto& item : edges_list) {
    auto& edges = *item.second;
    if (qk::ContainsKey(edges, a)) {
      auto& tmp = edges.at(a);
      for (auto& item2 : tmp) {
        output->push_back(item2.second.first);
      }
    }
  }
}

std::vector<StackOperation> AParseMachine::GetNextStackOps(const NFAState& s,
                                                           Alphabet a) const {
  std::vector<StackOperation> output;
  GetNextStackOps(s, a, &output);
  return output;
}

void AParseMachine::GetSpecialNextStates(
      const NFAState& s,
      Alphabet a,
      int e_non_termimal,
      qk::unordered_set<NFAState>* output) const {
  auto edges_list = GetSpecialOutgoingEdgesList(s);
  for (auto& item : edges_list) {
    auto& edges = *item.second;
    if (qk::ContainsKey(edges, a) &&
        qk::ContainsKey(edges.at(a), e_non_termimal)) {
      auto& tmp = edges.at(a).at(e_non_termimal).second;
      for (auto& item2 : tmp) {
        output->insert(item2.first.AddPrefixFromOther(s, item.first));
      }
    }
  }
}

void AParseMachine::GetParsingStream(const NFAState& source,
                                     Alphabet a,
                                     const NFAState& target,
                                     ParsingStream* output) const {
  auto edges_list = GetOutgoingEdgesList(source);
  for (auto& item : edges_list) {
    if (item.first > target.GetFullPathSize()) {
      break;
    }
    auto& edges = *item.second;
    auto new_target = target.GetSuffix(item.first);
    if (qk::ContainsKey(edges, a) &&
        qk::ContainsKey(edges.at(a), new_target)) {
      *output = edges.at(a).at(new_target);
      return;
    }
  }
}

void AParseMachine::GetSpecialParsingStream(const NFAState& source,
                                            Alphabet a,
                                            Alphabet e_non_termimal,
                                            const NFAState& target,
                                            ParsingStream* output) const {
  auto edges_list = GetSpecialOutgoingEdgesList(source);
  for (auto& item : edges_list) {
    auto& edges = *item.second;
    if (item.first > target.GetFullPathSize()) {
      break;
    }
    auto new_target = target.GetSuffix(item.first);
    if (qk::ContainsKey(edges, a) &&
        qk::ContainsKey(edges.at(a), e_non_termimal) &&
        qk::ContainsKey(edges.at(a).at(e_non_termimal).second, new_target)) {
      *output = edges.at(a).at(e_non_termimal).second.at(new_target);
      break;
    }
  }
}


qk::unordered_set<NFAState> AParseMachine::GetSpecialNextStates(
      const NFAState& s,
      Alphabet a,
      int enclosed_non_terminal) const {
  qk::unordered_set<NFAState> output;
  GetSpecialNextStates(s, a, enclosed_non_terminal, &output);
  return output;
}

std::unordered_set<Alphabet> AParseMachine::PossibleAlphabets(
    const NFAState& state) const {
  std::unordered_set<Alphabet> output;
  auto edges_list = GetOutgoingEdgesList(state);
  for (auto& item : edges_list) {
    qk::STLGetKeys(*item.second, &output);
  }
  auto sedges_list = GetSpecialOutgoingEdgesList(state);
  for (auto& item : sedges_list) {
    qk::STLGetKeys(*item.second, &output);
  }
  return output;
}

// ToDo(Mohit): So many copies of serialized_machine are created in
// import/export. Optimise it.
// Format Version - 3
std::string AParseMachine::Export() const {
  qk::OByteStream obs;
  uint32_t version = 3;
  obs << version << *this;
  return obs.str();
}

// Format Version - 3
bool AParseMachine::Import(const std::string& serialized_machine) {
  qk::IByteStream ibs;
  ibs.str(serialized_machine);
  uint32_t expected_version = 3, current_version;
  ibs >> current_version;
  if (current_version != expected_version) {
    return false;
  }
  ibs >> *this;
  // ToDo(Mohit): Validate.
  initialized = true;
  return true;
}

}  // namespace v2
}  // namespace aparse
