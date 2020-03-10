// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_APARSE_MACHINE_HPP_
#define _APARSE_APARSE_MACHINE_HPP_

static_assert(false, "use `aparse_machine_v3` instead");

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <string>

#include "quick/byte_stream.hpp"
#include "quick/unordered_map.hpp"

#include "aparse/common_headers.hpp"

#include "aparse/utils.hpp"
#include "aparse/regex.hpp"

namespace aparse {

/** AParseMachine is used for parsing AParse languages.
 *  AParseMachine is generated for a AParseGrammar using AParseMachineBuilder.
 */
struct AParseMachine {
  struct StackOperation {
    enum OperationType: uint8_t {PUSH, POP, NOOP};
    std::string DebugString() const;
    std::string GetTypeString() const;
    static std::string GetTypeString(OperationType type);
    void Serialize(quick::OByteStream&) const;
    void Deserialize(quick::IByteStream&);
    bool operator==(const StackOperation& o) const;

    OperationType type = NOOP;
    /** Applicable only for PUSH operation. */
    int32_t value = 0;
  };
  /** This is different from branching-alphabet in AParseGrammar.
   * BranchSymbolType is used for creating parse-tree from a alphabet stream.
   * Ideally it should be renamed to START_CHILD, END_CHILD */
  enum BranchSymbolType: uint8_t {START_BRANCH, END_BRANCH};

  static std::string GetBranchSymbolTypeString(BranchSymbolType t);
  /* A stream made of '(' and ')', (example: "((())(())" ), which will be
   * used while creating parsed-tree. Each '(' or ')' is labelled with
   * corrosponding regex's label (integer). */
  typedef std::vector<pair<BranchSymbolType, int32_t>> ParsingStream;
  struct NFA {
    struct EdgeLabel {
      void Serialize(quick::OByteStream&) const;
      void Deserialize(quick::IByteStream&);
      bool operator==(const EdgeLabel& o) const;

      ParsingStream parsing_stream;
      StackOperation stack_operation;
    };
    struct NFAState {
      void Serialize(quick::OByteStream&) const;
      void Deserialize(quick::IByteStream&);
      bool operator==(const NFAState& o) const;

      // Outgoing_Edges(current_state, Alphabet a, StackTopValue v) =
      //     if (v != undefined && ContainsKey(special_edges, (a,v)))
      //        then special_edges[(a,v)]
      //     else if (ContainsKey(edges, a))
      //        then edges[a]
      //     else
      //        []
      std::unordered_map<Alphabet, std::unordered_map<int32_t, EdgeLabel>> edges;
      qk::unordered_map<pair<Alphabet, int32_t>,
                    std::unordered_map<int32_t, EdgeLabel>> special_edges;
      ParsingStream parsing_stream;
      int32_t label = 0;  // optional field for external identification purpose.
    };
    std::string DebugString() const;
    void Serialize(quick::OByteStream&) const;
    void Deserialize(quick::IByteStream&);
    bool operator==(const NFA& o) const;

    std::vector<NFAState> states;
    // (Current State, Target State)
    //                     -->   (SecondLastFromTargetState -> ParsingStream)
    qk::unordered_map<pair<int32_t, int32_t>,
                      std::unordered_map<int32_t, ParsingStream>
                    > special_parsing_stream;
    int32_t start_state;
    std::unordered_set<int32_t> final_states;
  };
  struct DFA {
    struct DFAState {
      void Serialize(quick::OByteStream&) const;
      void Deserialize(quick::IByteStream&);
      bool operator==(const DFAState& o) const;

      std::unordered_map<Alphabet, pair<int32_t, StackOperation>> edges;
      qk::unordered_map<pair<Alphabet, int32_t>,
                    pair<int32_t, StackOperation>> special_edges;
      // Collection of NFA states, this DFAState is representing to.
      std::vector<int32_t> nfa_states;
      int32_t label = 0;  // optional field for external identification purpose.
    };
    std::string DebugString() const;
    void Serialize(quick::OByteStream&) const;
    void Deserialize(quick::IByteStream&);
    bool operator==(const DFA& o) const;

    std::vector<DFAState> states;
    int32_t start_state;
    std::unordered_set<int32_t> final_states;
    // For every final state f in DFA, there is a nfa's final state
    // g ∈ state[f].nfa_state. This is map of (f -> g) pairs.
    std::unordered_map<int32_t, int32_t> nfa_final_state_map;
  };
  struct BackTracking {
    void Serialize(quick::OByteStream&) const;
    void Deserialize(quick::IByteStream&);
    bool operator==(const BackTracking& o) const;

    // back(ns1, ds1, Transaction) : If ns1 is current NFA state, then calculate
    //                               previous NFA state, knowing that ds1 is
    //                               previous DFA state and DFA Transaction t
    //                               occured on ds1.
    //                               Transaction = (Alphabet, StackOp)
    // =
    //  if t.sop.type == NOOP || POP
    //     output = Anyof(NFAStates(ds1) ∩ IncomingNFAStates(ns1, t.alphabet));
    //     if t.sop.type == POP:
    //         TrackingStack.push((output, ns1));
    //     return output;
    //  else if t.sop.type == PUSH
    //     top = TrackingStack.pop();
    //     valid_nfa_states = list of x s.t. nfa_states[top.first]
    //                              .special_edges[(_, x)].contains(top.second)
    //     return Anyof(NFAStates(ds1) ∩ valid_nfa_states)
    //  --------------------------------
    //
    //
    // (ns1, ds1, alphabet) key of this map stores the output of
    // (NFAStates(ds1) ∩ IncomingNFAStates(ns1, alphabet)), precomputed in
    // advance for all possible combinations.
    qk::unordered_map<std::tuple<int32_t, int32_t, Alphabet>, int32_t> map1;
    // (ds1, a, b) key of this map stores the output of
    // back(_, ds1, {_, PUSH, _}), when TrackingStack.top = (a,b)
    qk::unordered_map<std::tuple<int32_t, int32_t, int32_t>, int32_t> map2;
    // (s1, sf, st) key means, .... ToDo(Mohit): Add comment here.
    qk::unordered_map<std::tuple<int32_t, int32_t, int32_t>,
                      ParsingStream> jump_parsing_stream;
  };
  std::string DebugString() const;
  std::string ShortDebugString() const;
  void Serialize(quick::OByteStream&) const;
  void Deserialize(quick::IByteStream&);
  bool operator==(const AParseMachine& o) const;
  std::string Export() const;
  bool Import(const std::string& serialized_machine);

  NFA nfa;
  DFA dfa;
  BackTracking back_tracking;
};


}  // namespace aparse


#endif  // _APARSE_APARSE_MACHINE_HPP_
