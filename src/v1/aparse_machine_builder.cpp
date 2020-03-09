// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/aparse_machine_builder_v2.hpp"

#include <algorithm>
#include <functional>

#include <quick/unordered_map.hpp>
#include <quick/unordered_set.hpp>
#include <quick/stl_utils.hpp>

namespace aparse {



void AParseMachineBuilder::Build(AParseMachine* output) {
  Step1BuildInternalGrammar();
  PreProcess2();
  CreateFiniteMachine();
  AddStackTransactions();
  BackTrackingPreprocess();
  PostProcess1();
  Export(output);
}


AParseMachine AParseMachineBuilder::Build() {
  AParseMachine output;
  Build(&output);
  return output;
}

void AParseMachineBuilder::Step1BuildInternalGrammar() {
  grammar.Validate();
  std::unordered_map<Alphabet, Alphabet> ba_map;
  std::unordered_map<Alphabet, Alphabet> ba_inverse_map;
  for (auto& item : grammar.branching_alphabets) {
    ba_map.insert(item);
    ba_inverse_map[item.second] = item.first;
  }
  auto lIsBAStart = [&](const Regex& r) {
    return (r.type == Regex::ATOMIC && ContainsKey(ba_map, r.alphabet));
  };
  auto lIsBAEnd = [&](const Regex& r, Alphabet start_ba) {
    return (r.type == Regex::ATOMIC && ba_map.at(start_ba) == r.alphabet);
  };
  std::unordered_map<int, Regex> rules;
  int max_non_terminal = 0;
  std::unordered_map<int, std::unordered_set<int>> repeated_non_terminals;
  for (int i = 0; i < grammar.rules.size(); i++) {
    repeated_non_terminals[grammar.rules[i].first].insert(i);
    max_non_terminal = std::max(max_non_terminal, grammar.rules[i].first);
  }
  int dummy_nt_index_counter = max_non_terminal + 1;
  for (auto& item : repeated_non_terminals) {
    if (item.second.size() > 1) {
      Regex r(Regex::UNION);
      for (auto rule_number : item.second) {
        int non_terminal = dummy_nt_index_counter++;
        rules[non_terminal] = grammar.rules[rule_number].second;
        rules[non_terminal].label = non_terminal;
        igrammar.regex_label_to_original_rule_number_mapping[non_terminal]
                                                               = rule_number;
        r.children.emplace_back(Regex(non_terminal));
      }
      rules[item.first] = std::move(r);
    } else {
      int rule_number = *item.second.begin();
      rules[item.first] = grammar.rules[rule_number].second;
      rules[item.first].label = item.first;
      igrammar.regex_label_to_original_rule_number_mapping[item.first]
                                                               = rule_number;
    }
  }
  int new_pa_counter = 100000;  // ToDo(Mohit): need to fix this.
  std::unordered_map<int, int> pa_map;
  std::function<void(Regex& regex)> lRemoveBA = [&](Regex& regex) {
    switch (regex.type) {
      case Regex::ATOMIC:
      case Regex::EPSILON: break;
      case Regex::UNION:
      case Regex::KPLUS:
      case Regex::KSTAR:
        for (auto& child : regex.children) {
          lRemoveBA(child);
        }
        break;
      case Regex::CONCAT: {
        auto& children = regex.children;
        if (false) {
          // vector<pair<int, vector<Regex>>> stack;
          // stack.resize(1);
          // for (int i = 0; i < children.size(); i++) {
          //   auto& child = children[i];
          //   if (child.type == Regex::ATOMIC
          //         && qk::ContainsKey(ba_map, child.alphabet)) {
          //     stack.resize(stack.size()+1);
          //     stack.back().first = child.alphabet;
          //   } else if (child.type == Regex::ATOMIC
          //         && qk::ContainsKey(ba_inverse_map, child.alphabet)) {
          //     auto& back = stack.back();
          //     if (back.second.size() == 1) {
          //       auto& regex = back.second[0];
          //       if (regex.type == Regex::ATOMIC
          //           && regex.alphabet >= alphabet_size) {
          //         stack.at(stack.size()-2).second.emplace_back(
          //           Regex(Regex::ATOMIC, regex.alphabet));
          //         int non_terminal;
          //         if (regex.alphabet < alphabet_size) {
          //           non_terminal = dummy_nt_index_counter++
          //           rules.emplace_back(make_pair(non_terminal,
          //                                        std::move(regex)));
          //         } else {
          //           non_terminal = regex.alphabet;
          //         }
          //       }
          //     }
          //     stack.pop();
          //   }
        }
        for (int i = 0; i < children.size(); i++) {
          auto& child = children[i];
          if (lIsBAStart(child) &&
               i+2 < children.size() &&
               children[i+1].type == Regex::ATOMIC &&
               ContainsKey(rules, children[i+1].alphabet) &&
               lIsBAEnd(children[i+2], child.alphabet)
              ) {
            auto pa = children[i+1].alphabet;
            auto ba_c = children[i+2].alphabet;
            igrammar.sub_regex_map[pa].branch_start_alphabet = child.alphabet;
            igrammar.sub_regex_map[pa].branch_close_alphabet = ba_c;

            int new_pa = new_pa_counter++;
            pa_map[new_pa] = pa;
            children[i+1].alphabet = new_pa;
            children.erase(children.begin()+i);
            children.erase(children.begin()+i+1);
          }
        }
        for (auto& child : regex.children) {
          lRemoveBA(child);
        }
        if (children.size() == 1) {
          regex = std::move(children[0]);
        }
        break;
      }
      default: assert(false);
    }
  };
  for (auto& rule : rules) {
    lRemoveBA(rule.second);
  }

  std::function<void(const Regex&, Regex*)> lExpand;
  lExpand = [&](const Regex& input, Regex*output) {
    switch (input.type) {
      case Regex::UNION:
      case Regex::CONCAT:
      case Regex::KSTAR:
      case Regex::KPLUS: {
        output->label = input.label;
        output->type = input.type;
        output->children.resize(input.children.size());
        for (int i = 0; i < input.children.size(); i++) {
          lExpand(input.children[i], &output->children[i]);
        }
        break;
      }
      case Regex::ATOMIC: {
        if (ContainsKey(pa_map, input.alphabet)) {
          *output = input;
          output->alphabet = pa_map.at(input.alphabet);
        } else if (ContainsKey(rules, input.alphabet)) {
          lExpand(rules[input.alphabet], output);
        } else {
          *output = input;
        }
        break;
      }
      case Regex::EPSILON: *output = input; break;
      default: assert(false); break;
    }
  };
  lExpand(rules.at(grammar.main_non_terminal), &igrammar.main_regex);
  for (auto& item : igrammar.sub_regex_map) {
    lExpand(rules.at(item.first), &item.second.regex);
  }
}



void AParseMachineBuilder::PreProcess2() {
  // Step-1: Make sure (regex.label > 0) is set for all the supporting-regex.
  // It will be needed for identification purpose.
  std::function<int(const Regex&)> lMaxRegexLabel = [&](const Regex& regex) {
    int output = regex.label;
    for (auto& child : regex.children) {
      output = std::max(output, lMaxRegexLabel(child));
    }
    return output;
  };
  auto lMaxGrammerLabel = [&](const InternalGrammar& igrammar) {
    int output = lMaxRegexLabel(igrammar.main_regex);
    for (auto& sub_regex : igrammar.sub_regex_map) {
      output = std::max(output, lMaxRegexLabel(sub_regex.second.regex));
    }
    return output;
  };
  int max_regex_label = std::max(0, lMaxGrammerLabel(igrammar));
  for (auto& sub_regex : igrammar.sub_regex_map) {
    int new_label = ++max_regex_label;
    original_regex_label_mapping[new_label] = sub_regex.second.regex.label;
    label_to_pa_mapping[new_label] = sub_regex.first;
    sub_regex.second.regex.label = new_label;
  }
}

void AParseMachineBuilder::PostProcess1() {
  auto lCleanParsingStream = [&](const ParsingStream& old_ps) {
    ParsingStream new_ps;
    for (auto& item : old_ps) {
      if (not ContainsKey(original_regex_label_mapping, item.second)) {
        new_ps.push_back(item);
      } else if (original_regex_label_mapping[item.second] > 0) {
        new_ps.push_back({item.first,
                          original_regex_label_mapping[item.second]});
      }
    }
    return new_ps;
  };
  for (int i = 0; i < nfa.states.size(); i++) {
    nfa.states[i].parsing_stream = lCleanParsingStream(
                                      nfa.states[i].parsing_stream);
    for (auto& item : nfa.states[i].edges) {
      for (auto& item2 : item.second) {
        item2.second.parsing_stream = lCleanParsingStream(
                                        item2.second.parsing_stream);
      }
    }
    for (auto& item : nfa.states[i].special_edges) {
      for (auto& item2 : item.second) {
        item2.second.parsing_stream = lCleanParsingStream(
                                        item2.second.parsing_stream);
      }
    }
  }
  for (auto& item : jump_parsing_stream) {
    item.second = lCleanParsingStream(item.second);
  }
}


// Copies all the outgoing edges of state s1 to state s2 as well.
void AParseMachineBuilder::AddEdges(int s1, int s2) {
  const auto& state1 = nfa.states[s1];
  auto& state2 = nfa.states[s2];
  for (auto& item: state1.edges) {
    for (auto& item2: item.second) {
      if (not ContainsKey(state2.edges[item.first], item2.first)) {
        nfa.states[item2.first].incoming_edges[s2].insert(item.first);
        state2.edges[item.first][item2.first].parsing_stream =
          Utils::STLJoinVector(state2.parsing_stream,
                               item2.second.parsing_stream);
      }
    }
  }
}

using SubSubNFA = AParseMachineBuilder::SubSubNFA;

SubSubNFA AParseMachineBuilder::MergeEquivalentFinalState(
      const SubSubNFA& ss_nfa) {
  SubSubNFA output;
  output.start_state = ss_nfa.start_state;
  auto final_states = ss_nfa.final_states;
  std::unordered_set<int> fs_to_remove;
  for (auto fs: final_states) {
    if (fs != ss_nfa.start_state && nfa.states[fs].incoming_edges.size() == 0) {
      fs_to_remove.insert(fs);
    }
  }
  final_states = qk::SetMinus(final_states, fs_to_remove);

  qk::unordered_map<ParsingStream, vector<int>> equivalent_fs_map;
  for (auto fs: final_states) {
    if (nfa.states[fs].edges.size() == 0) {
      equivalent_fs_map[nfa.states[fs].parsing_stream].push_back(fs);
    } else {
      output.final_states.insert(fs);
    }
  }
  for (auto& item: equivalent_fs_map) {
    output.final_states.insert(item.second[0]);
    if (item.second.size() > 1) {
      auto& v = item.second;
      for (int i = 1; i < v.size(); i++) {
        for (auto& item2: nfa.states[v[i]].incoming_edges) {
          for (auto alphabet: item2.second) {
            auto& source_state = nfa.states[item2.first];
            source_state.edges[alphabet][v[0]] = source_state.edges[alphabet][v[i]];
            nfa.states[v[0]].incoming_edges[item2.first].insert(alphabet);
            source_state.edges[alphabet].erase(v[i]);
          }
        }
      }
    }
  }
  return output;
}



AParseMachineBuilder::SubNFA AParseMachineBuilder::BuildNFA(const Regex& regex) {
  SubNFA output;
  auto& states = nfa.states;
  output.offset = nfa.states.size();
  std::function<SubSubNFA(const Regex&)> lNFABuilder = [&](const Regex& regex) {  // recursive
    SubSubNFA output;
    int ni_ofst = states.size(); // node-index-offset
    switch(regex.type) {
      case Regex::EPSILON: {
        states.push_back(InternalNFAState());
        output.start_state = ni_ofst;
        output.final_states.insert(ni_ofst);
        break;
      }
      case Regex::ATOMIC: {
        states.push_back(InternalNFAState());
        states.push_back(InternalNFAState());
        nfa.states[ni_ofst].edges[regex.alphabet][ni_ofst+1];
        nfa.states[ni_ofst+1].incoming_edges[ni_ofst].insert(regex.alphabet);
        output.start_state = ni_ofst;
        output.final_states.insert(ni_ofst+1);
        break;
      }
      case Regex::UNION: {
        states.push_back(InternalNFAState());
        output.start_state = ni_ofst;
        if (helpers::IsAllRegexChildrenAtomic(regex)) {  // Optimisation
          states.push_back(InternalNFAState());
          for (auto& child: regex.children) {
            nfa.states[ni_ofst].edges[child.alphabet][ni_ofst+1];
            nfa.states[ni_ofst+1].incoming_edges[ni_ofst].insert(child.alphabet);
          }
          output.final_states.insert(ni_ofst+1);
        } else {
          bool is_start_state_also_final = false;
          int accepting_ss_index = 0; // accepting_start_state_index
          for (auto& child: regex.children) {
            SubSubNFA snfa = lNFABuilder(child);
            Utils::STLInsertToSet(snfa.final_states, &output.final_states);
            AddEdges(snfa.start_state, output.start_state);
            if (not is_start_state_also_final &&
                ContainsKey(snfa.final_states, snfa.start_state)) {
              is_start_state_also_final = true;
              accepting_ss_index = snfa.start_state;
            }
          }
          if(is_start_state_also_final) {
            output.final_states.insert(output.start_state);
            nfa.states[output.start_state].parsing_stream =
                                   nfa.states[accepting_ss_index].parsing_stream;
          }
        }
        output = MergeEquivalentFinalState(output);
        break;
      }
      case Regex::CONCAT: {
        std::unordered_set<int> previous_final_states;
        for (int i=0; i < regex.children.size(); i++) {
          SubSubNFA snfa = lNFABuilder(regex.children[i]);
          if (i == 0) {
            output.start_state = snfa.start_state;
          } else {
            for (int fs: previous_final_states) {
              AddEdges(snfa.start_state, fs);
            }
          }
          if (i>0 && ContainsKey(snfa.final_states, snfa.start_state)) {
            for (int fs: previous_final_states) {
              qk::InsertToVector(nfa.states[snfa.start_state].parsing_stream,
                                &nfa.states[fs].parsing_stream);
            }
            Utils::STLInsertToSet(snfa.final_states, &previous_final_states);
          } else {
            previous_final_states = snfa.final_states;
          }
        }
        output.final_states = previous_final_states;
        break;
      }
      case Regex::KSTAR:
      case Regex::KPLUS: {
        states.push_back(InternalNFAState());
        output.start_state = ni_ofst;
        SubSubNFA snfa = lNFABuilder(regex.children[0]);
        output.final_states = snfa.final_states;
        for (auto fs: snfa.final_states) {
          AddEdges(snfa.start_state, fs);
        }
        AddEdges(snfa.start_state, output.start_state);
        if (regex.type == Regex::KSTAR) {
          output.final_states.insert(output.start_state);
        } else if (ContainsKey(snfa.final_states, snfa.start_state)) {
          output.final_states.insert(output.start_state);
          states[output.start_state].parsing_stream =
                                       nfa.states[snfa.start_state].parsing_stream;
        }
        break;
      }
      default:
        assert(false);
    }
    if (regex.label > 0) {
      for (int fs: output.final_states) {
        nfa.states[fs].parsing_stream.push_back(
          make_pair(AParseMachine::END_BRANCH, regex.label));
      }
      for (auto& edge: nfa.states[output.start_state].edges) {
        for (auto& item2: edge.second) {
          item2.second.parsing_stream.push_front(
            make_pair(AParseMachine::START_BRANCH, regex.label));
        }
      }
      if (ContainsKey(output.final_states, output.start_state)) {
        nfa.states[output.start_state].parsing_stream.push_front(
          make_pair(AParseMachine::START_BRANCH, regex.label));
      }
    }
    return output;
  };
  SubSubNFA snfa = lNFABuilder(regex);
  output.start_state = snfa.start_state;
  output.final_states = snfa.final_states;
  output.number_states = nfa.states.size() - output.offset;
  return ReduceNFA(output);
}



AParseMachineBuilder::SubNFA AParseMachineBuilder::ReduceNFA(
    AParseMachineBuilder::SubNFA& snfa) {
  SubNFA output;
  int& offset = snfa.offset;
  vector<bool> visited(nfa.states.size()-offset, false);
  std::function<void(int)> lDFS = [&](int u) {
    visited[u-offset] = true;
    for (auto &item : nfa.states[u].edges) {
      for (auto& item2 : item.second) {
        if (!visited[item2.first - offset]) {
          lDFS(item2.first);
        }
      }
    }
  };
  lDFS(snfa.start_state);
  vector<InternalNFAState> new_states;
  vector<int> new_position(visited.size());
  for (int i = 0; i < visited.size(); i++) {
    if (visited[i]) {
      new_position[i] = new_states.size();
      new_states.push_back(nfa.states[offset+i]);
    }
  }
  auto lNewPos = [&](int old_pos) {
    return offset+new_position[old_pos - offset];
  };
  output.start_state = lNewPos(snfa.start_state);
  std::unordered_set<int> new_final_states;
  for (const int& fs : snfa.final_states) {
    if (visited[fs-offset]) {
      new_final_states.insert(lNewPos(fs));
    }
  }
  for (auto& state : new_states) {
    decltype(state.edges) new_edges;
    for (auto &item : state.edges) {
      for (auto &item2 : item.second) {
        new_edges[item.first][lNewPos(item2.first)] = item2.second;
      }
    }
    state.edges = new_edges;
    APARSE_DEBUG_ASSERT(state.special_edges.size() == 0);
  }
  nfa.states.resize(offset);
  nfa.states.insert(nfa.states.end(), new_states.begin(), new_states.end());
  output.final_states = new_final_states;
  output.offset = offset;
  output.number_states = new_states.size();
  return output;
}

AParseMachineBuilder::SubDFA AParseMachineBuilder::BuildDFA(
    const AParseMachineBuilder::SubNFA& snfa) {
  SubDFA output;
  output.start_state = output.offset = dfa.states.size();
  qk::unordered_map<set<int>, int> dfa_index_map;
  qk::unordered_set<set<int>> cur_states;
  auto lAddDfaState = [&](const set<int> state) {
    dfa_index_map[state] = dfa.states.size();
    dfa.states.resize(dfa.states.size()+1);
    cur_states.insert(state);
  };
  lAddDfaState(set<int>({snfa.start_state}));
  while (!cur_states.empty()) {
    auto &top = *cur_states.begin();
    std::unordered_map<Alphabet, set<int>> next_states;
    for (int nfa_state : top) {
      for (auto& item : nfa.states[nfa_state].edges) {
        for (auto& item2 : item.second) {
          next_states[item.first].insert(item2.first);
        }
      }
    }
    uint dfa_index_of_top = dfa_index_map[top];
    for (auto& kv : next_states) {
      if (not aparse::ContainsKey(dfa_index_map, kv.second)) {
        lAddDfaState(kv.second);
      }
      dfa.states[dfa_index_of_top].edges[kv.first].first
                                               = dfa_index_map[kv.second];
    }
    dfa.states[dfa_index_of_top].nfa_states = Utils::STLContainerToVector(top);
    for (int nfa_state : top) {
      if (ContainsKey(snfa.final_states, nfa_state)) {
        output.final_states.insert(dfa_index_of_top);
        dfa.nfa_final_state_map[dfa_index_of_top] = nfa_state;
        break;
      }
    }
    cur_states.erase(top);
  }
  output.number_states = dfa.states.size() - output.offset;
  return output;
}

int AParseMachineBuilder::CompositeSubRegexStore::Add(
    const set<EnclosedNonTerminal>& pa_set) {
  if (not aparse::ContainsKey(csr_map, pa_set)) {
    int csr_index = csr_map[pa_set] = csr_list.size();
    csr_list.push_back(CompositeSubRegex(pa_set));
    return csr_index;
  }
  return csr_map[pa_set];
}

std::unordered_set<int> AParseMachineBuilder::ExtractPseudoAlphabets(
    AParseMachineBuilder::SubNFA& snfa,
    AParseMachineBuilder::SubDFA& sdfa) {
  std::unordered_set<int> output;  // Collection of CSR's indexes.
  for (int i = snfa.offset; i < snfa.offset + snfa.number_states; i++) {
    for (auto& item : nfa.states[i].edges) {
      if (item.first >= grammar.alphabet_size) {
        int pa = item.first;
        for (auto& item2 : item.second) {
          nfa.pseudo_edges[i][pa].first[item2.first]
                                                = item2.second.parsing_stream;
        }
      }
    }
    for (auto& item : nfa.pseudo_edges[i]) {
      nfa.states[i].edges.erase(item.first);
    }
  }
  for (int i = sdfa.offset; i < sdfa.offset+sdfa.number_states; i++) {
    for (auto& edge : dfa.states[i].edges) {
      if (edge.first >= grammar.alphabet_size) {
        EnclosedNonTerminal pa = edge.first;
        Alphabet ba =  igrammar.sub_regex_map[pa].branch_start_alphabet;
        dfa.pseudo_edges[i][ba].first[pa] = edge.second.first;
      }
    }
    for (auto& item : dfa.pseudo_edges[i]) {
      for (auto& item2 : item.second.first) {
        dfa.states[i].edges.erase(item2.first);
      }
    }
    if (qk::ContainsKey(dfa.pseudo_edges, i)) {
      for (auto& item : dfa.pseudo_edges[i]) {
        set<EnclosedNonTerminal> pa_set;
        qk::STLGetKeys(item.second.first, &pa_set);
        int csr_index = csr_store.Add(pa_set);
        output.insert(csr_index);
        item.second.second = csr_index;
        csr_store.Get(csr_index).incoming_dfa_states.insert(i);
      }
      for (int nfa_state : dfa.states[i].nfa_states) {
        if (qk::ContainsKey(nfa.pseudo_edges, nfa_state)) {
          for (auto& item : nfa.pseudo_edges[nfa_state]) {
            EnclosedNonTerminal pa = item.first;
            Alphabet ba = igrammar.sub_regex_map[pa].branch_start_alphabet;
            item.second.second.insert(dfa.pseudo_edges[i][ba].second);
          }
        }
      }
    }
  }
  return output;
}

void AParseMachineBuilder::BuildCSR(int csr_index) {
  auto& csr = csr_store.Get(csr_index);
  APARSE_DEBUG_ASSERT(csr.pa_set.size() > 0);
  csr.branch_close_alphabet = igrammar.sub_regex_map[*csr.pa_set.begin()]
                                                      .branch_close_alphabet;
  csr.branch_start_alphabet = igrammar.sub_regex_map[*csr.pa_set.begin()]
                                                      .branch_start_alphabet;
  Regex new_regex(Regex::UNION);
  for (int pa : csr.pa_set) {
    new_regex.children.push_back(igrammar.sub_regex_map[pa].regex);
  }
  // cout << "new_regex = " << new_regex.DebugString() << endl;
  csr.snfa = BuildNFA(new_regex);
  csr.sdfa = BuildDFA(csr.snfa);
  for (int fs : csr.snfa.final_states) {
    // Assert(nfa.states[fs].parsing_stream.size() > 0)
    int regex_label = nfa.states[fs].parsing_stream.back().second;
    csr.nfa_final_states[label_to_pa_mapping[regex_label]].push_back(fs);
  }
  for (int fs : csr.sdfa.final_states) {
    int nfa_fs = dfa.nfa_final_state_map[fs];
    // Assert(nfa.states[nfa_fs].parsing_stream.size() > 0);
    int regex_label = nfa.states[nfa_fs].parsing_stream.back().second;
    csr.dfa_final_states[label_to_pa_mapping[regex_label]].push_back(fs);
  }
}

void AParseMachineBuilder::CreateFiniteMachine() {
  nfa.main_snfa = BuildNFA(igrammar.main_regex);
  dfa.main_sdfa = BuildDFA(nfa.main_snfa);
  auto pending_csr_set = ExtractPseudoAlphabets(nfa.main_snfa, dfa.main_sdfa);
  std::unordered_set<int> built_csr_set;
  while (pending_csr_set.size() > 0) {
    int csr_index = *pending_csr_set.begin();
    if (not ContainsKey(built_csr_set, csr_index)) {
      BuildCSR(csr_index);
      auto sdfa2 = csr_store.Get(csr_index).sdfa;
      auto snfa2 = csr_store.Get(csr_index).snfa;
      auto new_csr_list = ExtractPseudoAlphabets(snfa2, sdfa2);
      pending_csr_set.insert(new_csr_list.begin(), new_csr_list.end());
      built_csr_set.insert(csr_index);
    }
    pending_csr_set.erase(csr_index);
  }
}

void AParseMachineBuilder::AddStackTransactions() {
  for (auto& item : dfa.pseudo_edges) {
    const int& dfa_state = item.first;
    for (auto& item2 : item.second) {
      auto& csr = csr_store.Get(item2.second.second);
      auto& sdfa = csr.sdfa;
      for (auto& item3 : item2.second.first) {
        dfa.states[item.first].edges[item2.first]
          = make_pair(sdfa.start_state,
                      StackOperation({StackOperation::PUSH, item.first}));
        for (int fs : csr.dfa_final_states[item3.first]) {
          dfa.states[fs].special_edges[make_pair(csr.branch_close_alphabet,
                                                 item.first)]
            = make_pair(item3.second, StackOperation({StackOperation::POP}));
        }
      }
    }
    for (auto& nfa_state : dfa.states[dfa_state].nfa_states) {
      if (ContainsKey(nfa.pseudo_edges, nfa_state)) {
        for (auto& item2 : nfa.pseudo_edges[nfa_state]) {
          const EnclosedNonTerminal& pa = item2.first;
          Alphabet ba = igrammar.sub_regex_map[pa].branch_start_alphabet;
          Alphabet ba2 = igrammar.sub_regex_map[pa].branch_close_alphabet;
          auto& csr = csr_store.Get(item.second[ba].second);
          nfa.states[nfa_state].edges[ba][csr.snfa.start_state] = {
            {},
            {StackOperation::PUSH, nfa_state}};
          for (auto& nfa_fs : csr.nfa_final_states[pa]) {
            for (auto& item3 : item2.second.first) {
              const int& ts = item3.first;
              jump_parsing_stream[make_tuple(nfa_state, nfa_fs, ts)]
                                                        = item3.second;
              nfa.states[nfa_fs].special_edges[make_pair(ba2, nfa_state)][ts]
                 = {
                    nfa.states[nfa_fs].parsing_stream,
                    {StackOperation::POP}
                  };
              back_tracking.map2[make_tuple(dfa_state, nfa_fs, ts)] = nfa_state;
            }
          }
        }
      }
    }
  }
}

void AParseMachineBuilder::BackTrackingPreprocess() {
  for (int i = 0; i < dfa.states.size(); i++) {
    for (auto& nfa_state : dfa.states[i].nfa_states) {
      for (auto& item : nfa.states[nfa_state].edges) {
        for (auto& item2 : item.second) {
          int ts = item2.first;
          back_tracking.map1[std::make_tuple(ts, i, item.first)] = nfa_state;
        }
      }
      for (auto& item : nfa.states[nfa_state].special_edges) {
        for (auto& item2 : item.second) {
          int ts = item2.first;
          back_tracking.map1[make_tuple(ts, i, item.first.first)] = nfa_state;
        }
      }
    }
  }
}

void AParseMachineBuilder::Export(AParseMachine* output_machine) {
  auto& output = *output_machine;
  output.nfa.start_state = nfa.main_snfa.start_state;
  output.nfa.final_states = nfa.main_snfa.final_states;
  output.nfa.states.resize(nfa.states.size());
  for (int i = 0; i < nfa.states.size(); i++) {
    for (auto& item : nfa.states[i].edges) {
      for (auto& item2 : item.second) {
        output.nfa.states[i].edges[item.first][item2.first] = {
          Utils::STLContainerToVector(item2.second.parsing_stream),
          item2.second.stack_operation
        };
      }
    }
    for (auto& item : nfa.states[i].special_edges) {
      for (auto& item2 : item.second) {
        output.nfa.states[i].special_edges[item.first][item2.first] = {
          Utils::STLContainerToVector(item2.second.parsing_stream),
          item2.second.stack_operation
        };
      }
    }
    output.nfa.states[i].parsing_stream = Utils::STLContainerToVector(
                                                nfa.states[i].parsing_stream);
  }
  output.dfa.start_state = dfa.main_sdfa.start_state;
  output.dfa.final_states = dfa.main_sdfa.final_states;
  output.dfa.states.resize(dfa.states.size());
  // ToDo(Mohit): Consider the actual final states only (in main FSM)
  // output.dfa.nfa_final_state_map = dfa.nfa_final_state_map;
  for (auto item: output.dfa.final_states) {
    output.dfa.nfa_final_state_map[item] = dfa.nfa_final_state_map[item];
  }
  for (int i = 0; i < dfa.states.size(); i++) {
    auto& state = dfa.states[i];
    output.dfa.states[i] = {state.edges,
                            state.special_edges,
                            state.nfa_states,
                            state.label};
  }
  output.back_tracking = back_tracking;
  for (auto& item : jump_parsing_stream) {
    output.back_tracking.jump_parsing_stream[item.first]
                                 = Utils::STLContainerToVector(item.second);
  }
}

string AParseMachineBuilder::CompositeSubRegexStore::DebugString() {
  APARSE_DEBUG_ASSERT(false, "Implement this method");
  // cout << "Number of CSR = " << csr_list.size() << endl;
  // for (int i = 0; i < csr_list.size(); i++) {
  //   auto& csr = csr_list[i];
  //   cout << "csr " << i << " : " << endl;
  //   cout << "pa_set = " << csr.pa_set << endl;
  //   cout << "NFA final_states : " << csr.nfa_final_states << endl;
  //   cout << "DFA final_states : " << csr.dfa_final_states << endl;
  //   cout << "------------------" << endl;
  // }
  return "";
}

string AParseMachineBuilder::InternalGrammar::DebugString() const {
  std::ostringstream oss;
  oss << "{\n  main_regex = " << main_regex.DebugString() << "\n";
  for (auto& item : sub_regex_map) {
    oss << "  sub_regex[" << item.first << "] : " << "Open("
        << item.second.branch_start_alphabet << "), Close("
        << item.second.branch_close_alphabet << "), Regex = "
        << item.second.regex.DebugString() << "\n";
  }
  oss << "}\n";
  return oss.str();
}


string AParseMachineBuilder::DebugString() const {
  APARSE_DEBUG_ASSERT(false, "Implement this method");

  // // std::ostringstream oss;
  // cout << "NFA's pseudo_edges: " << endl;
  // for (auto& item: nfa.pseudo_edges) {
  //   for (auto& item2: item.second) {
  //     for (auto& item3: item2.second.first) {
  //       cout << "(State " << item.first << ", PA " << item2.first << ", Ts "
  //            << item3.first << ")" << endl;
  //     }
  //   }
  // }
  // cout << "DFA's pseudo_edges: " << endl;
  // for (auto& item: dfa.pseudo_edges) {
  //   for (auto& item2: item.second) {
  //     for (auto& item3: item2.second.first) {
  //       cout << "(State " << item.first << ", BA " << item2.first << ", PA "
  //            << item3.first << ", TS " << item3.second << ", csr_index "
  //            << item2.second.second << ")" << endl;
  //     }
  //   }
  // }
  // for (auto& state: nfa.states) {
  //   cout << "NFA's special_edges.size() = " << state.special_edges.size()
  //        << endl;
  // }
  // for (auto& state: dfa.states) {
  //   cout << "DFA's special_edges.size() = " << state.special_edges.size()
  //        << endl;
  // }
  // csr_store.DebugString();
  // cout << "original_regex_label_mapping = " << original_regex_label_mapping
  //      << endl;
  // cout << "label_to_pa_mapping = " << label_to_pa_mapping << endl;
  return "";
}

}  // namespace aparse


