// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_SRC_V2_APARSE_MACHINE_BUILDER_HPP_
#define APARSE_SRC_V2_APARSE_MACHINE_BUILDER_HPP_

#include <utility>
#include <list>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>

#include "quick/unordered_map.hpp"
#include "quick/debug_stream_decl.hpp"

#include "aparse/common_headers.hpp"
#include "aparse/aparse_grammar.hpp"
#include "src/v2/aparse_machine.hpp"

#include "src/v2/internal_aparse_grammar.hpp"

namespace aparse {
namespace v2 {
namespace aparse_machine_builder_impl {

class AParseMachineBuilder {
 public:
  explicit AParseMachineBuilder(const AParseGrammar& grammar)
      : grammar(grammar) {}
  AParseMachine Build();
  void Build(AParseMachine*);
  void DebugStream(qk::DebugStream& ds) const;  // NOLINT

  using NFAState = AParseMachine::NFAState;
  template<typename T> using NFAStateMap = qk::unordered_map<NFAState, T>;
  using NFAStateSet = qk::unordered_set<NFAState>;
  template<typename T> using AlphabetMap = std::unordered_map<Alphabet, T>;
  using ParsingStream = std::list<pair<AParseMachine::BranchSymbolType, int>>;

  using OutgoingEdges = AlphabetMap<NFAStateMap<ParsingStream>>;
  using SpecialOutgoingEdges = AParseMachine::SpecialOutgoingEdges;
  struct NFA;
  using NFAMap = std::unordered_map<int, NFA>;
  using RegexRulesMap = std::unordered_map<int, Regex>;

  struct SubNFA {
    void DebugStream(qk::DebugStream& ds) const;  // NOLINT
    NFAState start_state;
    NFAStateMap<ParsingStream> final_states;
  };

  struct NFA {
    void DebugStream(qk::DebugStream& ds) const;  // NOLINT

    // Map(source_state -> Map(alphabet -> Map(target_state -> parsing_stream)))
    NFAStateMap<OutgoingEdges> edges;
    NFAStateMap<SpecialOutgoingEdges> special_edges;
    NFAStateMap<ParsingStream> final_states;
    NFAState start_state;
    // Map(TargetState -> Map(Alphabet -> Set of source states which are
    //                                    pointing to target state on the given
    //                                    alphabet))
    NFAStateMap<AlphabetMap<NFAStateSet>> local_incoming_edges;
  };

  // - Assumes that dependency NFAs (i.e. NFAs corrosponding to dependency
  //    non-terminals) are already built and stored in `dependency_nfa_map`.
  class NFABuilder {
   public:
    NFABuilder(const NFAMap& dependency_nfa_map,
               const RegexRulesMap& rules,
               std::unordered_map<int, int>* nfa_instance_map,
               int* state_number_counter)
      : nfa_map(dependency_nfa_map),
        rules(rules),
        nfa_instance_map(nfa_instance_map),
        state_number_counter(state_number_counter) {}

    void Build(const Regex& regex, NFA* output);

   protected:
    void RemoveUnreachableStates();

    // Get all the outgoing edges from @state, combining both local and global
    // edges and insert in @output.
    void GetOutgoingEdges(const NFAState& state,
                          OutgoingEdges* output) const;

    // Calculate outgoing edges using 'GetOutgoingEdges' and adds it to output.
    // Additionally add @local_incoming_edges in NFA. Additionally left-append
    // the given parsing stream in each outgoing edge.
    void AddEdges(const NFAState& state,
                  const ParsingStream& ps,
                  const NFAState& new_state);

    SubNFA BuildSubNFA(const Regex& regex);
    void BuildSubNFA(const Regex& regex, SubNFA* output);
    void BuildEpsilonSubNFA(const Regex& regex, SubNFA* output);
    void BuildAtomicSubNFA(const Regex& regex, SubNFA* output);
    void BuildUnionSubNFA(const Regex& regex, SubNFA* output);
    void BuildConcatSubNFA(const Regex& regex, SubNFA* output);
    void BuildKstarKPlusSubNFA(const Regex& regex, SubNFA* output);

    void WrapWithParsingStream(int label, SubNFA* snfa);

    void MergeEquivalentFinalState(SubNFA* snfa);

    // Assumes that NFA corrosponding to @non_terminal is already created.
    void GetNFAInstance(int non_terminal, SubNFA* output);


   private:
    const NFAMap& nfa_map;
    const RegexRulesMap& rules;
    // not owned.
    std::unordered_map<int, int>* nfa_instance_map;
    int* state_number_counter;
    NFA nfa;
  };

 public:
  InternalAParseGrammar igrammar;

 protected:
  void BuildNFAs();
  void AddStackOperations();
  void ExportToAParseMachine(AParseMachine* output) const;

  // Map(non-terminal -> NFA)
  std::unordered_map<int, NFA> nfa_map;
  // Map(non-terminal -> number of instances of their NFAs, referenced in other
  //                     NFA's)
  std::unordered_map<int, int> nfa_instance_map;
  const AParseGrammar& grammar;
};

}  // namespace aparse_machine_builder_impl
using aparse_machine_builder_impl::AParseMachineBuilder;
}  // namespace v2
}  // namespace aparse


#endif  // APARSE_SRC_V2_APARSE_MACHINE_BUILDER_HPP_
