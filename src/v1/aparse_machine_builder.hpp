// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_APARSE_MACHINE_BUILDER_V2_HPP_
#define _APARSE_APARSE_MACHINE_BUILDER_V2_HPP_

#include <utility>
#include <list>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>

#include "quick/unordered_map.hpp"

#include "aparse/common_headers.hpp"
#include "aparse/aparse_grammar.hpp"
#include "aparse/aparse_machine.hpp"

namespace aparse {

class AParseMachineBuilder {
 public:
  explicit AParseMachineBuilder(const AParseGrammar& grammar)
      : grammar(grammar) {}
  AParseMachine Build();
  void Build(AParseMachine*);
  string DebugString() const;
  struct SubSubNFA {
    int start_state;
    std::unordered_set<int> final_states;
  };

 private:
  using StackOperation = AParseMachine::StackOperation;
  using ParsingStream = std::list<pair<AParseMachine::BranchSymbolType, int>>;

  struct EdgeLabel {
    ParsingStream parsing_stream;
    StackOperation stack_operation;
  };


  struct InternalNFAState {
    // Alphabet -> ( Target State -> EdgeLable )
    std::unordered_map<Alphabet, std::unordered_map<int, EdgeLabel>> edges;
    // Source State -> (Set of Alphabet which are pointing to this state)
    std::unordered_map<int, std::unordered_set<Alphabet>> incoming_edges;
    // (Alphabet, stack-top) -> ( Target State -> EdgeLabel )
    qk::unordered_map<pair<Alphabet, int>,
                  std::unordered_map<int, EdgeLabel>
                 > special_edges;
    ParsingStream parsing_stream;
    int label;  // optional field for external identification purpose.
  };

  struct InternalDFAState {
    // Alphabet -> {1. Target State, 2. Stack Ops}
    std::unordered_map<Alphabet, pair<int, StackOperation>> edges;
    // (Alphabet, StackTop) -> {1. Target State, 2. Stack Ops}
    qk::unordered_map<pair<Alphabet, int>,
                  pair<int, StackOperation>
                  > special_edges;
    vector<int> nfa_states;
    int label;  // optional field for external identification purpose.
  };

  struct SubNFA {
    int start_state;
    std::unordered_set<int> final_states;
    int offset;
    int number_states;
  };

  struct SubDFA {
    int start_state;
    std::unordered_set<int> final_states;
    int offset;
    int number_states;
  };

  struct NFA {
    SubNFA main_snfa;
    // int start_state;
    // std::unordered_set<int> final_states;
    vector<InternalNFAState> states;
    // NFA State ->
    //     PseudoAlphabet ->
    //         1. Map(TargetState -> ParsingStream)
    //         2. set<int> csr_index_list
    typedef std::unordered_map<int, ParsingStream> tmp_type1;
    typedef pair<tmp_type1, std::unordered_set<int>> tmp_type2;
    typedef std::unordered_map<PseudoAlphabet, tmp_type2> tmp_type3;
    std::unordered_map<int, tmp_type3> pseudo_edges;
  };

  struct DFA {
    SubDFA main_sdfa;
    vector<InternalDFAState> states;
    // Corrospinding final state of NFA for this final state of DFA.
    std::unordered_map<int, int> nfa_final_state_map;
    // DFA State ->
    //    Branch-Alphabet ->
    //           1. Map(PseudoAlphabet -> TargetState)
    //           2. csr_index;
    typedef pair<std::unordered_map<PseudoAlphabet, int>, int> tmp_type1;
    std::unordered_map<int, std::unordered_map<Alphabet, tmp_type1>> pseudo_edges;
  };

  struct CompositeSubRegex {
    SubDFA sdfa;
    SubNFA snfa;
    set<PseudoAlphabet> pa_set;
    Alphabet branch_start_alphabet;
    Alphabet branch_close_alphabet;
    std::unordered_set<int> incoming_dfa_states;
    // Map (PseudoAlphabet -> List of Corrosponding Final States)
    std::unordered_map<PseudoAlphabet, vector<int>> nfa_final_states;
    std::unordered_map<PseudoAlphabet, vector<int>> dfa_final_states;
    explicit CompositeSubRegex(const set<PseudoAlphabet>& pa_set)
      : pa_set(pa_set) {}
  };

  struct CompositeSubRegexStore {
    vector<CompositeSubRegex> csr_list;
    qk::unordered_map<set<int>, int> csr_map;
    CompositeSubRegex& Get(int index) {
      return csr_list[index];}
    CompositeSubRegex& Get(const set<int>& pa_set) {
      return csr_list[csr_map[pa_set]];}
    int Add(const set<PseudoAlphabet>& pa_set);
    void IndexOf(const set<PseudoAlphabet>& pa_set);
    string DebugString();
  };

  const AParseGrammar& grammar;

  std::unordered_map<PseudoAlphabet, int> original_regex_label_mapping;
  std::unordered_map<int, PseudoAlphabet> label_to_pa_mapping;
  CompositeSubRegexStore csr_store;
  NFA nfa;
  DFA dfa;
  AParseMachine::BackTracking back_tracking;
  qk::unordered_map<std::tuple<int, int, int>,
                     ParsingStream> jump_parsing_stream;

  SubNFA BuildNFA(const Regex& regex);
  SubDFA BuildDFA(const SubNFA& snfa);

  void CreateFiniteMachine();

  // Add all the outgoing edges of state1 to state2 as well.
  void AddEdges(int s1, int s2);
  void AddStackTransactions();
  void Export(AParseMachine* output);
  void PreProcess2();
  void PostProcess1();
  // Reducing NFA to optimally-smallest NFA is a NP-hard problem, but
  // this method implements some heuristics:
  // 1. Delete the unreachable states.
  // 2. ToDo(Mohit): Delete the states, which have no future. i.e. there is no
  //    final-state reachable from that state.
  // 3. ToDo(Mohit): Remove `NOLINT` and make it ReduceNFA(SubNFA* snfa)
  SubNFA ReduceNFA(SubNFA& snfa);  // NOLINT
  SubSubNFA MergeEquivalentFinalState(const SubSubNFA& ss_nfa);
  // ToDo(Mohit): Make it SubNFA* and SubDFA* and remove `NOLINT`
  std::unordered_set<int> ExtractPseudoAlphabets(SubNFA& snfa,  // NOLINT
                                            SubDFA& sdfa);  // NOLINT
  void BuildCSR(int csr_index);
  void BackTrackingPreprocess();

 public:
  // This method is declared without implementing. Extrernal user can implement
  // this for testing purpose and can use internal members of this class.
  void Debug();
  struct InternalGrammar {
    InternalGrammar() {}
    struct SubRegex {
      Regex regex;
      Alphabet branch_start_alphabet;
      Alphabet branch_close_alphabet;
    };
    Regex main_regex;
    // Mapping from enclosed alphabets to corrosponding Regex.
    std::unordered_map<EnclosedsAlphabet, SubRegex> sub_regex_map;
    std::unordered_map<int, int> regex_label_to_original_rule_number_mapping;
    string DebugString() const;
  };
  InternalGrammar igrammar;
  void Step1BuildInternalGrammar();

};


}  // namespace aparse


#endif  // _APARSE_APARSE_MACHINE_BUILDER_V2_HPP_
