// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/v2/core_parser.hpp"

#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include <sstream>

#include "aparse/error.hpp"
#include "quick/debug.hpp"
#include "quick/stl_utils.hpp"

namespace aparse {
namespace v2 {
using NFAState = AParseMachine::NFAState;
using StackOperation = AParseMachine::StackOperation;
using NFAStateSet = qk::unordered_set<NFAState>;
using ParsingStream = AParseMachine::ParsingStream;
template<typename T> using NFAStateMap = qk::unordered_map<NFAState, T>;

NFAStateMap<NFAState> CurrentState::NextStatesMap(
      Alphabet alphabet,
      const AParseMachine& machine) const {
  NFAStateMap<NFAState> next_states_map;
  for (auto& s : nfa_states) {
    auto ns = machine.GetNextStates(s, alphabet);
    for (auto& item : ns) {
      next_states_map.emplace(item, s);
    }
  }
  return next_states_map;
}

// Next states across special_edges, filtered on a subset of
// @enclosed_non_terminals.
// Map(TargetState -> Pair(1. SourceState
//                         2. ent on which this transition is taken))
NFAStateMap<std::pair<NFAState, int>> CurrentState::SpecialNextStates(
      Alphabet alphabet,
      const AParseMachine& machine,
      const std::unordered_set<int>& enclosed_non_terminals) const {
  NFAStateMap<std::pair<NFAState, int>> output;
  for (auto& s : nfa_states) {
    for (auto ent : enclosed_non_terminals) {
      auto next_states = machine.GetSpecialNextStates(s, alphabet, ent);
      for (auto& s2 : next_states) {
        output[s2] = make_pair(s, ent);
      }
    }
  }
  return output;
}

std::pair<StackOperation::OperationType, unordered_map<int, NFAState>>
CurrentState::NextStackOps(
      Alphabet alphabet,
      const AParseMachine& machine) const {
  std::unordered_map<int, NFAState> source_state_map;
  auto op_type = StackOperation::NOP;
  for (auto& s : nfa_states) {
    auto stack_ops = machine.GetNextStackOps(s, alphabet);
    for (auto& item : stack_ops) {
      source_state_map.insert({item.enclosed_non_terminal, s});
      op_type = item.type;
    }
  }
  return make_pair(op_type, source_state_map);
}

bool CurrentState::IsFinal(const AParseMachine& machine) const {
  for (auto& s : nfa_states) {
    if (machine.IsFinalState(s)) {
      return true;
    }
  }
  return false;
}

CoreParser::CoreParser(const qk::AbstractType* machine) {
  this->SetAParseMachine(machine);
}

// Is Idempotent ? : Yes
void CoreParser::SetAParseMachine(const qk::AbstractType* machine) {
  APARSE_ASSERT(machine != nullptr);
  this->machine = static_cast<const AParseMachine*>(machine);
  APARSE_ASSERT(this->machine->initialized);
  this->Reset();
}

void StackFrame::DebugStream(qk::DebugStream& ds) const {
  ds << "alphabet = " << alphabet << "\n"
     << "nfa_states = " << nfa_states;
}

void CurrentState::DebugStream(qk::DebugStream& ds) const {
  ds << nfa_states;
}

// deprecated method.
unordered_set<Alphabet> CoreParser::PossibleAlphabets(int k) const {
  return PossibleAlphabets();
}

unordered_set<Alphabet> CoreParser::PossibleAlphabets() const {
  unordered_set<Alphabet> output;
  for (auto& state : current_state.nfa_states) {
    auto alphabets = machine->PossibleAlphabets(state);
    qk::InsertToSet(alphabets, &output);
  }
  return output;
}

void CoreParser::BackTracker::DebugStream(qk::DebugStream& ds) const {
  ds << "nfa_states_map = " << nfa_states_map << "\n"
     << "pull_op_map = " << pull_op_map;
}


void CoreParser::DebugStream(qk::DebugStream& ds) const {
  ds << "current_state = " << current_state << "\n"
     << "stack = " << stack
     << "\n"
     << "back_tracker = " << back_tracker << "\n"
     << "is_valid_path_so_far = " << is_valid_path_so_far;
}

void CoreParser::Reset() {
  is_valid_path_so_far = true;
  current_state = CurrentState({machine->start_state});
  stack.clear();
  stack_op_list.clear();
  stream.clear();
  back_tracker.nfa_states_map.clear();
  back_tracker.pull_op_map.clear();
  // static_assert(sizeof(*this) == 216, "Update CoreParser::Reset method");
}

const vector<Alphabet>& CoreParser::GetStream() const {
  return stream;
}

bool CoreParser::Feed(const vector<Alphabet>& stream) {
  for (int i = 0; i < stream.size(); i++) {
    if (not Feed(stream[i])) {
      return false;
    }
  }
  return true;
}

void CoreParser::FeedOrDie(const vector<Alphabet>& stream) {
  for (int i = 0; i < stream.size(); i++) {
    FeedOrDie(stream[i]);
  }
}

bool CoreParser::Feed(const vector<Alphabet>& stream, Error* error) {
  for (int i = 0; i < stream.size(); i++) {
    if (not Feed(stream[i], error))
      return false;
  }
  return true;
}

void CoreParser::FeedOrDie(Alphabet a) {
  if (not Feed(a)) {
    throw Error(Error::PARSING_ERROR_INVALID_TOKENS)
                .Position({stream.size(), 1})
                .PossibleAlphabets(PossibleAlphabets())();
  }
}

bool CoreParser::Feed(Alphabet alphabet, Error* error) {
  if (not Feed(alphabet)) {
    *error = Error(Error::PARSING_ERROR_INVALID_TOKENS)
                .Position({stream.size(), 1})
                .PossibleAlphabets(PossibleAlphabets())();
    return false;
  }
  return true;
}

bool CoreParser::CanFeed(Alphabet alphabet) const {
  APARSE_ASSERT(false, "ToDo(Mohit): Implement it");
}

bool CoreParser::Feed(Alphabet alphabet) {
  if (not is_valid_path_so_far) return false;
  auto stack_op = current_state.NextStackOps(alphabet, *machine);
  int feed_index = stream.size();
  if (stack_op.first == StackOperation::PUSH) {
    stack.emplace_back(alphabet, current_state.nfa_states);
    current_state.nfa_states.clear();
    for (auto& x : stack_op.second) {
      current_state.nfa_states.insert(
          machine->enclosed_subnfa_map.at(x.first).start_state);
    }
  } else if (stack_op.first == StackOperation::POP) {
    APARSE_DEBUG_ASSERT(stack.size() > 0);
    auto& stack_frame = stack.back();
    std::unordered_set<int> enclosed_non_terminals;
    qk::STLGetKeys(stack_op.second, &enclosed_non_terminals);
    auto next_states_map = CurrentState(stack_frame.nfa_states)
                              .SpecialNextStates(stack_frame.alphabet,
                                                 *machine,
                                                 enclosed_non_terminals);
    auto& back_track_info = back_tracker.pull_op_map[feed_index];
    for (auto& item : next_states_map) {
      back_track_info[item.first] =
          make_tuple(item.second.first,
          item.second.second,
          stack_op.second.at(item.second.second));
    }
    current_state.nfa_states.clear();
    qk::STLGetKeys(next_states_map, &current_state.nfa_states);
    stack.pop_back();
  } else {
    auto next_states_map = current_state.NextStatesMap(alphabet, *machine);
    if (next_states_map.size() > 0) {
      current_state.nfa_states.clear();
      back_tracker.nfa_states_map[feed_index] = next_states_map;
      qk::STLGetKeys(next_states_map, &current_state.nfa_states);
    } else {
      is_valid_path_so_far = false;
      return false;
    }
  }
  stack_op_list.push_back(stack_op.first);
  stream.push_back(alphabet);
  return true;
}

bool CoreParser::IsFinal() const {
  return is_valid_path_so_far && current_state.IsFinal(*machine);
}

void CoreParser::ParseOrDie(CoreParseNode* output) {
  if (not Parse(output)) {
    throw Error(Error::PARSING_ERROR_INCOMPLETE_TOKENS)
                .PossibleAlphabets(PossibleAlphabets())();
  }
}

bool CoreParser::Parse(CoreParseNode* output, Error* error) {
  if (not Parse(output)) {
    *error = Error(Error::PARSING_ERROR_INCOMPLETE_TOKENS)
                .PossibleAlphabets(PossibleAlphabets())();
    return false;
  }
  return true;
}

namespace {

// Given a stream of '(' and ')'  [eg: "()()()(())"], it constructs the tree.
void ConstructTree(const vector<ParsingStream>& parsing_stream,
                   CoreParseNode* output) {
  vector<CoreParseNode*> branching_stack;
  output->start = 0;
  output->end = parsing_stream.size() - 1;
  branching_stack.push_back(output);
  for (int i=0; i < parsing_stream.size(); i++) {
    for (auto& ps : parsing_stream[i]) {
      if (ps.first == AParseMachine::BRANCH_START_MARKER) {
        branching_stack.back()->children.push_back(
                                            CoreParseNode({ps.second, i}));
        branching_stack.push_back(&branching_stack.back()->children.back());
      } else {  // ps.first == BRANCH_END_MARKER
        branching_stack.back()->end = i;
        branching_stack.pop_back();
        APARSE_DEBUG_ASSERT(branching_stack.size() > 0);
      }
    }
  }
  APARSE_DEBUG_ASSERT(branching_stack.size() == 1);
  APARSE_DEBUG_ASSERT(output->children.size() > 0);
}

}  // namespace


bool CoreParser::Parse(CoreParseNode* output) {
  NFAState final_state;
  bool has_final_state = false;
  for (auto& s : current_state.nfa_states) {
    if (machine->IsFinalState(s)) {
      final_state = s;
      has_final_state = true;
      break;
    }
  }
  if (not has_final_state) {
    return false;
  }
  vector<AParseMachine::ParsingStream> parsing_stream(1+stream.size());
  parsing_stream[stream.size()] = machine->final_states.at(final_state);
  NFAState cur = final_state;
  vector<std::tuple<NFAState, int, NFAState>> construction_stack;
  for (int i = static_cast<int>(stream.size()) - 1; i >= 0; i--) {
    auto stack_op = stack_op_list.at(i);
    switch (stack_op) {
      case StackOperation::PUSH: {
        auto& tmp = construction_stack.back();
        machine->GetSpecialParsingStream(std::get<0>(tmp),
                                         stream.at(i),
                                         std::get<1>(tmp),
                                         std::get<2>(tmp),
                                         &parsing_stream[i]);
        cur = std::get<0>(tmp);
        construction_stack.pop_back();
        break;
      }
      case StackOperation::POP: {
        auto& tmp = back_tracker.pull_op_map.at(i).at(cur);
        construction_stack.push_back(make_tuple(std::get<0>(tmp),
                                                std::get<1>(tmp), cur));
        cur = std::get<2>(tmp);
        parsing_stream[i] = machine->enclosed_subnfa_map.at(
                                    std::get<1>(tmp)).final_states.at(cur);
        break;
      }
      case StackOperation::NOP: {
        auto new_cur = back_tracker.nfa_states_map.at(i).at(cur);
        machine->GetParsingStream(new_cur,
                                  stream.at(i),
                                  cur,
                                  &parsing_stream[i]);
        cur = new_cur;
        break;
      }
      default: assert(false);
    }
  }
  ConstructTree(parsing_stream, output);
  return true;
}

}  // namespace v2
}  // namespace aparse
