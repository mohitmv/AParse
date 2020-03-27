// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/v2/aparse_machine_builder.hpp"

#include <algorithm>
#include <functional>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include <quick/unordered_map.hpp>
#include <quick/unordered_set.hpp>
#include <quick/stl_utils.hpp>
#include <quick/debug_stream.hpp>

#include <quick/debug.hpp>

namespace aparse {
namespace v2 {
namespace aparse_machine_builder_impl {

using StackOperation = AParseMachine::StackOperation;
using NFABuilder = AParseMachineBuilder::NFABuilder;
using NFA =  AParseMachineBuilder::NFA;
using OutgoingEdges = AParseMachineBuilder::OutgoingEdges;
using SpecialOutgoingEdges = AParseMachine::SpecialOutgoingEdges;
using SubNFA = AParseMachineBuilder::SubNFA;
using NFAState = AParseMachineBuilder::NFAState;
template<typename T> using NFAStateMap = qk::unordered_map<NFAState, T>;
using ParsingStream = AParseMachineBuilder::ParsingStream;

void SubNFA::DebugStream(qk::DebugStream& ds) const {
  ds << "start_state = " << start_state << "\n"
     << "final_states = " << final_states;
}

AParseMachine AParseMachineBuilder::Build() {
  AParseMachine output;
  Build(&output);
  return output;
}

void NFABuilder::GetOutgoingEdges(const NFAState& state,
                                  OutgoingEdges* output) const {
  if (qk::ContainsKey(nfa.edges, state)) {
    *output = nfa.edges.at(state);
  }
  auto p = state.GetFullPath();
  for (int i = 0; i < state.GetFullPathSize(); i++) {
    NFAState state_id = state.GetSuffix(i+1);
    auto& tmp = nfa_map.at(p[i].first).edges;
    if (qk::ContainsKey(tmp, state_id)) {
      const auto& outgoing_edges = tmp.at(state_id);
      for (auto& item : outgoing_edges) {
        for (auto& item2 : item.second) {
          (*output)[item.first][item2.first.AddPrefixFromOther(state, i+1)]
                     = item2.second;
        }
      }
    }
  }
}

void NFABuilder::AddEdges(const NFAState& state,
                          const ParsingStream& ps,
                          const NFAState& new_state) {
  OutgoingEdges o;
  GetOutgoingEdges(state, &o);
  auto& new_edges = nfa.edges[new_state];
  for (auto& item : o) {
    for (auto& item2 : item.second) {
      if (item2.first.IsLocal()) {
        nfa.local_incoming_edges[item2.first][item.first].insert(new_state);
      }
      new_edges[item.first][item2.first] = qk::ListJoin(ps, item2.second);
    }
  }
}

void NFABuilder::Build(const Regex& regex, NFA* output) {
  SubNFA snfa = BuildSubNFA(regex);
  nfa.start_state = std::move(snfa.start_state);
  nfa.final_states = std::move(snfa.final_states);
  RemoveUnreachableStates();
  *output = std::move(nfa);
}

void NFABuilder::GetNFAInstance(int non_terminal, SubNFA* output) {
  int instance_id = (*nfa_instance_map)[non_terminal]++;
  auto& nfa = nfa_map.at(non_terminal);
  output->start_state = nfa.start_state.AddPrefix(non_terminal, instance_id);
  for (auto& item : nfa.final_states) {
    output->final_states[item.first.AddPrefix(non_terminal, instance_id)]
                 = item.second;
  }
}

void NFABuilder::BuildEpsilonSubNFA(const Regex& regex, SubNFA* output) {
  output->start_state = NFAState((*state_number_counter)++);
  output->final_states[output->start_state];
}

void NFABuilder::BuildAtomicSubNFA(const Regex& regex, SubNFA* output) {
  if (qk::ContainsKey(rules, regex.alphabet)) {  // non-terminals
    int non_terminal = regex.alphabet;
    if (rules.at(non_terminal).type == Regex::ATOMIC) {
      BuildSubNFA(rules.at(non_terminal), output);
    } else {
      GetNFAInstance(non_terminal, output);
    }
  } else {  // terminals or enclosed-non-terminals.
    output->start_state = NFAState((*state_number_counter)++);
    auto final_state = NFAState((*state_number_counter)++);
    nfa.edges[output->start_state][regex.alphabet][final_state];
    nfa.local_incoming_edges[final_state][regex.alphabet]
                                  .insert(output->start_state);
    output->final_states[final_state];
  }
}

void NFABuilder::BuildUnionSubNFA(const Regex& regex, SubNFA* output) {
  output->start_state = NFAState((*state_number_counter)++);
  bool is_start_state_also_final = false;
  for (auto& child : regex.children) {
    SubNFA snfa = BuildSubNFA(child);
    qk::InsertToMap(snfa.final_states, &output->final_states);
    AddEdges(snfa.start_state, ParsingStream(), output->start_state);
    if (not is_start_state_also_final &&
        qk::ContainsKey(snfa.final_states, snfa.start_state)) {
      is_start_state_also_final = true;
      output->final_states[output->start_state] =
                    snfa.final_states[snfa.start_state];
    }
  }
  MergeEquivalentFinalState(output);
}

void NFABuilder::BuildConcatSubNFA(const Regex& regex, SubNFA* output) {
  NFAStateMap<ParsingStream> previous_final_states;
  for (int i = 0; i < regex.children.size(); i++) {
    SubNFA snfa = BuildSubNFA(regex.children[i]);
    if (i == 0) {
      output->start_state = snfa.start_state;
    } else {
      for (auto& fs : previous_final_states) {
        AddEdges(snfa.start_state, fs.second, fs.first);
      }
    }
    if (i > 0 && ContainsKey(snfa.final_states, snfa.start_state)) {
      for (auto& fs : previous_final_states) {
        qk::InsertToVector(snfa.final_states[snfa.start_state],
                           &fs.second);
      }
      qk::InsertToMap(snfa.final_states, &previous_final_states);
    } else {
      previous_final_states = snfa.final_states;
    }
  }
  output->final_states = previous_final_states;
}


void NFABuilder::BuildKstarKPlusSubNFA(const Regex& regex, SubNFA* output) {
  output->start_state = NFAState((*state_number_counter)++);
  SubNFA snfa = BuildSubNFA(regex.children[0]);
  output->final_states = snfa.final_states;
  for (auto fs : snfa.final_states) {
    AddEdges(snfa.start_state, fs.second, fs.first);
  }
  AddEdges(snfa.start_state, ParsingStream(), output->start_state);
  if (regex.type == Regex::KSTAR) {
    output->final_states[output->start_state];
  } else if (ContainsKey(snfa.final_states, snfa.start_state)) {
    output->final_states[output->start_state] =
                                 snfa.final_states[snfa.start_state];
  }
}

void NFABuilder::WrapWithParsingStream(int label, SubNFA* snfa) {
  if (not snfa->start_state.IsLocal()) {
    NFAState new_start_state((*state_number_counter)++);
    ParsingStream ps;
    if (qk::ContainsKey(snfa->final_states, snfa->start_state)) {
      ps = snfa->final_states.at(snfa->start_state);
    }
    AddEdges(snfa->start_state, ps, new_start_state);
    snfa->start_state = new_start_state;
  }

  for (auto& item : snfa->final_states) {
    item.second.push_back(
      make_pair(AParseMachine::BRANCH_END_MARKER, label));
  }
  if (qk::ContainsKey(nfa.edges, snfa->start_state)) {
    for (auto& edge : nfa.edges.at(snfa->start_state)) {
      for (auto& item2 : edge.second) {
        item2.second.push_front(
          make_pair(AParseMachine::BRANCH_START_MARKER, label));
      }
    }
  }
  if (ContainsKey(snfa->final_states, snfa->start_state)) {
    snfa->final_states[snfa->start_state].push_front(
      make_pair(AParseMachine::BRANCH_START_MARKER, label));
  }
}

void NFABuilder::MergeEquivalentFinalState(SubNFA* snfa) {
  // local final states, with zero outgoing edges, united by unique parsing
  // stream.
  qk::unordered_map<ParsingStream, vector<NFAState>> local_final_states;
  for (auto& item : snfa->final_states) {
    auto& fs = item.first;
    if (fs.IsLocal() and not(qk::ContainsKey(nfa.edges, fs))) {
      local_final_states[item.second].push_back(fs);
    }
  }
  // For each unique parsing-stream, if there are multiple final states, then
  // combine them all together.
  for (auto& item : local_final_states) {
    auto& ps = item.first;
    auto& fs_list = item.second;
    if (fs_list.size() <= 1) {
      continue;
    }
    auto& fs0 = fs_list[0];
    for (int i = 1; i < fs_list.size(); i++) {
      auto& fs = fs_list[i];
      if (qk::ContainsKey(nfa.local_incoming_edges, fs)) {
        auto& i_edges = nfa.local_incoming_edges.at(fs);
        for (auto& item2 : i_edges) {
          Alphabet a = item2.first;
          for (auto& source_state : item2.second) {
            // invariant: fs != fs0
            nfa.edges.at(source_state).at(a)[fs0] = ps;
            nfa.edges.at(source_state).at(a).erase(fs);
            nfa.local_incoming_edges.at(fs0)[a].insert(source_state);
          }
        }
        nfa.local_incoming_edges.erase(fs);
      }
      snfa->final_states.erase(fs);
    }
  }
}

SubNFA NFABuilder::BuildSubNFA(const Regex& regex) {
  SubNFA output;
  BuildSubNFA(regex, &output);
  return output;
}

void NFABuilder::BuildSubNFA(const Regex& regex, SubNFA* output) {
  switch (regex.type) {
    case Regex::EPSILON: {
      BuildEpsilonSubNFA(regex, output);
      break;
    }
    case Regex::ATOMIC: {
      BuildAtomicSubNFA(regex, output);
      break;
    }
    case Regex::UNION: {
      BuildUnionSubNFA(regex, output);
      break;
    }
    case Regex::CONCAT: {
      BuildConcatSubNFA(regex, output);
      break;
    }
    case Regex::KSTAR:
    case Regex::KPLUS: {
      BuildKstarKPlusSubNFA(regex, output);
      break;
    }
    default:
      assert(false);
  }
  if (regex.label > 0) {
    WrapWithParsingStream(regex.label, output);
  }
}

void NFABuilder::RemoveUnreachableStates() {
  using NFAStateSet = qk::unordered_set<NFAState>;
  qk::unordered_map<pair<int, int>, NFAStateSet> instances_sources;
  for (auto& item : nfa.edges) {
    auto& source = item.first;
    if (not source.IsLocal()) {
      instances_sources[source.GetI(0)].insert(source);
    }
  }
  std::queue<NFAState> q;
  NFAStateSet pushed_states;
  qk::unordered_set<pair<int, int>> pushed_instances;
  q.push(nfa.start_state);
  pushed_states.insert(nfa.start_state);
  while (not q.empty()) {
    auto f = q.front();
    q.pop();
    if (qk::ContainsKey(nfa.edges, f)) {
      for (auto& item : nfa.edges.at(f)) {
        for (auto& item2 : item.second) {
          auto target = item2.first;
          if (target.IsLocal()) {
            if (not qk::ContainsKey(pushed_states, target)) {
              pushed_states.insert(target);
              q.push(target);
            }
          } else {
            auto instance = target.GetI(0);
            if (not qk::ContainsKey(pushed_instances, instance)) {
              pushed_instances.insert(instance);
              if (qk::ContainsKey(instances_sources, instance)) {
                for (auto& item3 : instances_sources.at(instance)) {
                  pushed_states.insert(item3);
                  q.push(item3);
                }
              }
            }
          }
        }
      }
    }
  }
  vector<NFAState> to_delete;
  for (auto& item : nfa.edges) {
    auto& source = item.first;
    if (not qk::ContainsKey(pushed_states, source)) {
      to_delete.push_back(source);
    }
  }
  for (auto& source : to_delete) {
    nfa.edges.erase(source);
    nfa.final_states.erase(source);
  }
}

void AParseMachineBuilder::BuildNFAs() {
  int state_number_counter = 0;
  for (auto nt : igrammar.topological_sorted_non_terminals) {
    if (nt == igrammar.main_non_terminal or
        igrammar.rules.at(nt).type != Regex::ATOMIC) {
      NFABuilder builder(nfa_map,
                         igrammar.rules,
                         &nfa_instance_map,
                         &state_number_counter);
      builder.Build(igrammar.rules.at(nt), &nfa_map[nt]);
    }
  }
  for (auto ent : igrammar.enclosed_non_terminals) {
    NFABuilder builder(nfa_map,
                       igrammar.rules,
                       &nfa_instance_map,
                       &state_number_counter);
    builder.Build(igrammar.enclosed_rules.at(ent), &nfa_map[ent]);
  }
}

void AParseMachineBuilder::Build(AParseMachine* output) {
  igrammar.Init(grammar);
  BuildNFAs();
  AddStackOperations();
  ExportToAParseMachine(output);
}

namespace {

void ExportParsingStreamMap(
    const InternalAParseGrammar& igrammar,
    const NFAStateMap<ParsingStream>& targets,
    NFAStateMap<AParseMachine::ParsingStream>* output) {
  for (auto& item : targets) {
    auto& ps = (*output)[item.first];
    for (auto& item2 : item.second) {
      auto& label_map = igrammar.regex_label_to_original_rule_number_mapping;
      auto rule_number = label_map.at(item2.second);
      ps.emplace_back(make_pair(item2.first, rule_number));
    }
  }
}

void ExportOutgoingEdges(const InternalAParseGrammar& igrammar,
                         const NFAStateMap<OutgoingEdges>& input,
                         NFAStateMap<AParseMachine::OutgoingEdges>* output) {
  for (auto& edge_item : input) {
    const NFAState& source_state = edge_item.first;
    for (auto& item : edge_item.second) {
      Alphabet a = item.first;
      auto& target_states = item.second;
      ExportParsingStreamMap(igrammar,
                             target_states,
                             &(*output)[source_state][a]);
    }
  }
}

}  // namespace

void AParseMachineBuilder::AddStackOperations() {
  for (auto& nfa_item : nfa_map) {
    auto& nfa = nfa_item.second;
    for (auto& edge_item : nfa.edges) {
      const NFAState& source_state = edge_item.first;
      std::unordered_set<int> ent_edges;
      for (auto& item : edge_item.second) {
        Alphabet a = item.first;
        if (qk::ContainsKey(igrammar.enclosed_non_terminals, a)) {
          Alphabet ba = igrammar.enclosed_ba_alphabet.at(a);
          auto& tmp = nfa.special_edges[source_state][ba][a];
          tmp.first.type = StackOperation::PUSH;
          tmp.first.enclosed_non_terminal = a;
          ExportParsingStreamMap(igrammar, item.second, &tmp.second);
          ent_edges.insert(a);
        }
      }
      for (auto& ent : ent_edges) {
        edge_item.second.erase(ent);
      }
    }
  }
  for (auto& ent : igrammar.enclosed_non_terminals) {
    auto& e_nfa = nfa_map.at(ent);
    Alphabet ba = igrammar.enclosed_ba_alphabet.at(ent);
    Alphabet closing_ba = igrammar.ba_map.at(ba);
    for (auto& item2 : e_nfa.final_states) {
      e_nfa.special_edges[item2.first][closing_ba][ent].first =
                {StackOperation::POP, ent};
    }
  }
}

void AParseMachineBuilder::ExportToAParseMachine(AParseMachine* output) const {
  auto lExportToNFALookupMap = [&](const NFA& nfa, int non_terminal) {
    for (auto& item : nfa.edges) {
      output->nfa_lookup_map[item.first] = non_terminal;
    }
    for (auto& item : nfa.special_edges) {
      output->nfa_lookup_map[item.first] = non_terminal;
    }
  };
  for (auto& item : nfa_map) {
    auto nt = item.first;
    auto& nfa = item.second;
    auto& output_nfa = output->nfa_map[nt];
    output_nfa.special_edges = nfa.special_edges;
    ExportOutgoingEdges(igrammar, nfa.edges, &output_nfa.edges);
    if (qk::ContainsKey(igrammar.enclosed_non_terminals, nt)) {
      output->enclosed_subnfa_map[nt].start_state = nfa.start_state;
      ExportParsingStreamMap(igrammar,
                             nfa.final_states,
                             &output->enclosed_subnfa_map[nt].final_states);
      lExportToNFALookupMap(nfa, nt);
    } else if (nt == igrammar.main_non_terminal) {
      output->start_state = nfa.start_state;
      ExportParsingStreamMap(igrammar,
                             nfa.final_states,
                             &output->final_states);
      lExportToNFALookupMap(nfa, nt);
    }
  }
  output->initialized = true;
}

void NFA::DebugStream(qk::DebugStream& ds) const {
  ds << "start_state = " << start_state << "\n"
     << "final_states = " << final_states << "\n"
     << "edges = " << edges << "\n"
     << "local_incoming_edges = " << local_incoming_edges;
}

void AParseMachineBuilder::DebugStream(qk::DebugStream& ds) const {
  ds << "igrammar = " << igrammar << "\n"
     << "nfa_map = " << nfa_map;
}

}  // namespace aparse_machine_builder_impl
}  // namespace v2
}  // namespace aparse
