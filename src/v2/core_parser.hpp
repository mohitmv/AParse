// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_SRC_V2_CORE_PARSER_HPP_
#define APARSE_SRC_V2_CORE_PARSER_HPP_

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <tuple>

#include <quick/unordered_set.hpp>
#include <quick/debug_stream_decl.hpp>

#include "aparse/error.hpp"
#include "src/abstract_core_parser.hpp"
#include "src/v2/aparse_machine.hpp"

namespace aparse {
namespace v2 {

struct StackFrame {
  using NFAState = AParseMachine::NFAState;
  using NFAStateSet = qk::unordered_set<NFAState>;
  StackFrame() = default;
  StackFrame(Alphabet a, const NFAStateSet& s): alphabet(a), nfa_states(s) {}
  void DebugStream(qk::DebugStream& ds) const;  // NOLINT
  Alphabet alphabet;
  NFAStateSet nfa_states;
};

struct CurrentState {
  using NFAState = AParseMachine::NFAState;
  using StackOperation = AParseMachine::StackOperation;
  using NFAStateSet = qk::unordered_set<NFAState>;
  template<typename T> using NFAStateMap = qk::unordered_map<NFAState, T>;
  CurrentState() = default;
  explicit CurrentState(const NFAStateSet& nfa_states)
  : nfa_states(nfa_states) {}
  NFAStateMap<NFAState> NextStatesMap(Alphabet alphabet,
                                      const AParseMachine& machine) const;
  std::pair<StackOperation::OperationType,
            unordered_map<int, NFAState>>
  NextStackOps(Alphabet alphabet, const AParseMachine& machine) const;
  NFAStateMap<std::pair<NFAState, int>> SpecialNextStates(
      Alphabet alphabet,
      const AParseMachine& machine,
      const std::unordered_set<int>& enclosed_non_terminals) const;

  bool IsFinal(const AParseMachine& machine) const;
  void DebugStream(qk::DebugStream& ds) const;  // NOLINT

  NFAStateSet nfa_states;
};


/** CoreParser the the main Parser, which parse a string and create ParseTree.
 *  CoreParser stores the const-reference of AParseMachine. Hence
 *  AParseMachine must live longer than CoreParser object.
 *  CoreParser is used by Parser, which is a shallow wrapper around CoreParser.
 *  To parse multiple string, client can either create different CoreParser
 *  objects or client can invoke Reset method and reuse the same CoreParser
 *  object. */
class CoreParser : public AbstractCoreParser {
 public:
  CoreParser() = default;
  explicit CoreParser(const qk::AbstractType* machine);
  void SetAParseMachine(const qk::AbstractType* machine);

  bool Parse(CoreParseNode* output);
  void ParseOrDie(CoreParseNode* output);
  bool Parse(CoreParseNode* output, Error* error);

  bool Feed(Alphabet alphabet);
  void FeedOrDie(Alphabet alphabet);
  bool Feed(Alphabet alphabet, Error* error);
  bool Feed(const vector<Alphabet>& stream);
  void FeedOrDie(const vector<Alphabet>& stream);
  bool Feed(const vector<Alphabet>& stream, Error* error);

  bool CanFeed(Alphabet alphabet) const;
  bool IsFinal() const;
  void Reset();
  const vector<Alphabet>& GetStream() const;
  unordered_set<Alphabet> PossibleAlphabets() const;
  unordered_set<Alphabet> PossibleAlphabets(int k) const;  // return k only.
  void DebugStream(qk::DebugStream&) const;  // NOLINT
  using NFAState = AParseMachine::NFAState;
  template<typename T> using NFAStateMap = qk::unordered_map<NFAState, T>;
  struct BackTracker {
    void DebugStream(qk::DebugStream& ds) const;  // NOLINT
    // feed_index -> Map(current_states -> their previous state)
    std::unordered_map<int, NFAStateMap<NFAState>> nfa_states_map;
    // This map is stored only for 'POP' stack operations.
    // feed_index -> Map(current_state ->
    //     Tuple(1. It's previous state among the states pushed in stack
    //           2. Recognized enclosed_non_terminal,
    //           3. It's previous state in enclose_nfa))
    std::unordered_map<int, NFAStateMap<std::tuple<NFAState,
                                                   int,
                                                   NFAState>>> pull_op_map;
  };

 private:
  using StackOperation = AParseMachine::StackOperation;
  const AParseMachine* machine = nullptr;
  // Invariant: Update the default values of these members in Reset method.
  bool is_valid_path_so_far = true;
  CurrentState current_state;
  vector<StackFrame> stack;
  vector<Alphabet> stream;
  vector<StackOperation::OperationType> stack_op_list;
  BackTracker back_tracker;
};

}  // namespace v2
}  // namespace aparse

#endif  // APARSE_SRC_V2_CORE_PARSER_HPP_
