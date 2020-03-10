// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_SRC_V2_APARSE_MACHINE_HPP_
#define APARSE_SRC_V2_APARSE_MACHINE_HPP_

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <string>

#include "quick/byte_stream.hpp"
#include "quick/unordered_map.hpp"
#include "quick/unordered_set.hpp"
#include "quick/debug_stream_decl.hpp"
#include "quick/utility.hpp"

#include "aparse/common_headers.hpp"

namespace aparse {
namespace v2 {


/** AParseMachine is used for parsing AParse languages.
 *  AParseMachine is generated for a AParseGrammar using AParseMachineBuilder.
 */
struct AParseMachine : public quick::AbstractType {
  struct NFAState {
    NFAState() {}
    explicit NFAState(int number): number(number) {}
    std::string DebugString() const;
    std::size_t GetHash() const;
    void Serialize(quick::OByteStream&) const;  // NOLINT
    void Deserialize(quick::IByteStream&);  // NOLINT
    bool operator==(const NFAState& other) const;
    bool operator!=(const NFAState& other) const;
    NFAState AddPrefix(int non_terminal, int nfa_id) const;
    NFAState AddPrefix(
        const vector<std::pair<int, int>>& specifiers) const;
    /** Consider only first @len specifiers. */
    vector<std::pair<int, int>> GetPrefix(int len) const;
    /** Exclude the first @len specifiers. */
    NFAState GetSuffix(int len) const;
    NFAState AddPrefixFromOther(const NFAState& other, int len) const;
    pair<int, int> GetI(int i) const;
    vector<pair<int, int>> GetFullPath() const;
    inline int GetNumber() const {return number;}
    int GetFullPathSize() const;
    bool IsLocal() const;

   private:
    int number;
    /** vector<pair<(non-terminal, instance-id)>> */
    std::vector<std::pair<int, int>> full_path;
  };

  struct StackOperation {
    enum OperationType: uint8_t {PUSH, POP, NOP};
    StackOperation() = default;
    StackOperation(OperationType type, int enclosed_non_terminal):
      type(type),
      enclosed_non_terminal(enclosed_non_terminal) {}
    void DebugStream(qk::DebugStream&) const;
    std::string GetTypeString() const;
    static std::string GetTypeString(OperationType type);
    void Serialize(quick::OByteStream&) const;
    void Deserialize(quick::IByteStream&);
    bool operator==(const StackOperation& o) const;

    OperationType type = NOP;
    int enclosed_non_terminal;
  };

  /** This is different from branching-alphabet in AParseGrammar.
   *  BranchSymbolType is used for creating parse-tree from a alphabet stream.
   *  Ideally it should be renamed to START_CHILD, END_CHILD  */
  enum BranchSymbolType: uint8_t {BRANCH_START_MARKER, BRANCH_END_MARKER};
  static std::string GetBranchSymbolTypeString(BranchSymbolType t);

  template<typename T> using NFAStateMap = qk::unordered_map<NFAState, T>;
  using NFAStateSet = qk::unordered_set<NFAState>;
  template<typename T> using AlphabetMap = std::unordered_map<Alphabet, T>;

  // A stream made of '(' and ')', (example: "((())(())" ), which will be
  // used while creating parsed-tree. Each '(' or ')' is labelled with
  // corrosponding regex's label (integer).
  using ParsingStream = std::vector<pair<BranchSymbolType, int32_t>>;

  using OutgoingEdges = AlphabetMap<NFAStateMap<ParsingStream>>;
  // map(branching-alphabet -> map(enclosed-non-terminal ->
  //          pair(1. StackOperation,
  //               2. map(target-state -> ParsingStream))))
  using SpecialOutgoingEdges = AlphabetMap<
                                  AlphabetMap<
                                      std::pair<
                                          StackOperation,
                                          NFAStateMap<ParsingStream>>>>;

  struct NFA {
    bool operator==(const NFA& other) const;
    void Serialize(quick::OByteStream&) const;
    void Deserialize(quick::IByteStream&);  // NOLINT

    void DebugStream(qk::DebugStream& ds) const;  // NOLINT

    NFAStateMap<OutgoingEdges> edges;
    NFAStateMap<SpecialOutgoingEdges> special_edges;
  };
  struct EnclosedSubNFA {
    void Serialize(quick::OByteStream&) const;  // NOLINT
    void Deserialize(quick::IByteStream&);  // NOLINT
    bool operator==(const EnclosedSubNFA& o) const;
    void DebugStream(qk::DebugStream& ds) const;  // NOLINT

    NFAStateMap<ParsingStream> final_states;
    NFAState start_state;
  };

  void GetNextStates(const NFAState& s,
                     Alphabet a,
                     qk::unordered_set<NFAState>* output) const;

  qk::unordered_set<NFAState> GetNextStates(const NFAState& s,
                                            Alphabet a) const;

  void GetNextStackOps(const NFAState& s,
                       Alphabet a,
                       std::vector<StackOperation>* output) const;

  std::vector<StackOperation> GetNextStackOps(const NFAState& s,
                                              Alphabet a) const;

  void GetSpecialNextStates(const NFAState& s,
                            Alphabet a,
                            int enclosed_non_terminal,
                            qk::unordered_set<NFAState>* output) const;

  qk::unordered_set<NFAState> GetSpecialNextStates(
        const NFAState& s,
        Alphabet a,
        int enclosed_non_terminal) const;

  bool IsFinalState(const NFAState& state) const;

  void DebugStream(qk::DebugStream& ds) const;  // NOLINT

  void GetParsingStream(const NFAState& source,
                        Alphabet a,
                        const NFAState& target,
                        ParsingStream* output) const;

  void GetSpecialParsingStream(const NFAState& source,
                               Alphabet a,
                               Alphabet e_non_termimal,
                               const NFAState& target,
                               ParsingStream* output) const;

  std::string DebugString() const;
  std::string ShortDebugString() const;
  void Serialize(quick::OByteStream&) const;  // NOLINT
  void Deserialize(quick::IByteStream&);  // NOLINT
  bool operator==(const AParseMachine& o) const;
  std::string Export() const;
  bool Import(const std::string& serialized_machine);
  std::unordered_set<Alphabet> PossibleAlphabets(const NFAState& state) const;

  std::unordered_map<int, NFA> nfa_map;
  NFAState start_state;
  NFAStateMap<ParsingStream> final_states;
  // `NFAState` is unique across all the NFAs. This is map from a
  // nfa_state -> index of the NFA having this nfa_state as local state.
  NFAStateMap<int> nfa_lookup_map;
  std::unordered_map<int, EnclosedSubNFA> enclosed_subnfa_map;
  bool initialized = false;

 private:
  // return Vector<Pair(1. number of suffixes stripped from NFAState,
  //                    2. Corrosponding outgoing edges)>
  vector<pair<int, const OutgoingEdges*>> GetOutgoingEdgesList(
      const NFAState& s) const;

  // return Vector<Pair(1. number of suffixes stripped from NFAState,
  //                    2. Corrosponding special outgoing edges)>
  vector<pair<int, const SpecialOutgoingEdges*>> GetSpecialOutgoingEdgesList(
      const NFAState& s) const;
};

qk::DebugStream& operator<<(qk::DebugStream& ds,
                            AParseMachine::BranchSymbolType bs);

}  // namespace v2
}  // namespace aparse

#endif  // APARSE_SRC_V2_APARSE_MACHINE_HPP_
