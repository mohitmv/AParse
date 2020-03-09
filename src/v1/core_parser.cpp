// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/core_parser.hpp"

#include <sstream>

#include "aparse/error.hpp"
#include "quick/debug.hpp"
#include "quick/stl_utils.hpp"

namespace aparse {


bool CoreParseNode::operator==(const CoreParseNode& rhs) const {
  return (label == rhs.label &&
            start == rhs.start &&
            end == rhs.end &&
            children == rhs.children);
}

string CoreParseNode::DebugString() const {
  auto lChildren = [&](const CoreParseNode* node) {
    vector<const CoreParseNode*> children;
    for (auto& item : node->children) {
      children.push_back(&item);
    }
    return children;
  };
  auto lValue = [&](const CoreParseNode* node) {
    std::ostringstream oss;
    oss << node->label << " [" << node->start << " - " << node->end << "] ";
    return oss.str();
  };
  return Utils::PrintPrettyTree(this, lChildren, lValue);
}

// Is Idempotent ? : Yes
void CoreParser::SetAParseMachine(const AParseMachine* machine) {
  this->machine = machine;
  this->current_state = machine->dfa.start_state;
  this->dfa_path.clear();
  this->dfa_path.push_back(machine->dfa.start_state);
}


vector<Alphabet> CoreParser::PossibleAlphabets(int k) const {
  vector<Alphabet> output;
  auto& state = machine->dfa.states[current_state];
  if (stack.size() > 0) {
    for (auto& item : state.special_edges) {
      if (k > 0) {
        if (item.first.second == stack.back()) {
          output.push_back(item.first.first);
          k--;
        }
      } else {
        break;
      }
    }
  }
  for (auto& item : state.edges) {
    if (k > 0) {
      output.push_back(item.first);
      k--;
    } else {
      break;
    }
  }
  return output;
}

string CoreParser::DebugString() const {
  std::ostringstream oss;
  oss << "current_state = " << current_state << "\n";
  oss << "stack = " << stack << "\n";
  oss << "dfa_path = " << dfa_path << "\n";
  oss << "is_valid_path_so_far = " << is_valid_path_so_far << "\n";
  oss << "stream.size() = " << stream.size() << "\n";
  return oss.str();
}

void CoreParser::Reset() {
  is_valid_path_so_far = true;
  current_state = machine->dfa.start_state;
  dfa_path.clear();
  dfa_path.push_back(machine->dfa.start_state);
  stack.clear();
  stream.clear();
  static_assert(sizeof(*this) == 88, "Update CoreParser::Reset method");
}

Alphabet CoreParser::GetAlphabet(int index) const {
  return stream[index].first;
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
                .PossibleAlphabets(PossibleAlphabets(10))();
  }
}

bool CoreParser::Feed(Alphabet alphabet, Error* error) {
  if (not Feed(alphabet)) {
    *error = Error(Error::PARSING_ERROR_INVALID_TOKENS)
                .Position({stream.size(), 1})
                .PossibleAlphabets(PossibleAlphabets(10))();
    return false;
  }
  return true;
}

bool CoreParser::CanFeed(Alphabet alphabet) const {
  APARSE_ASSERT(false, "ToDo(Mohit): Implement it");
}

bool CoreParser::Feed(Alphabet alphabet) {
  if (not is_valid_path_so_far) return false;
  auto& state = machine->dfa.states[current_state];
  pair<int, StackOperation> transaction;
  if (stack.size() > 0
        && qk::ContainsKey(state.special_edges,
                           make_pair(alphabet, stack.back()))) {
    transaction = state.special_edges.at(make_pair(alphabet, stack.back()));
  } else if (ContainsKey(state.edges, alphabet)) {
    transaction = state.edges.at(alphabet);
  } else {
    is_valid_path_so_far = false;
    return false;
  }
  switch (transaction.second.type) {
    case StackOperation::PUSH:
      stack.push_back(transaction.second.value); break;
    case StackOperation::POP: {
      APARSE_DEBUG_ASSERT(stack.size() > 0);
      stack.pop_back();
      break;
    }
    case StackOperation::NOOP: break;
    default: assert(false);
  }
  current_state = transaction.first;
  dfa_path.push_back(current_state);
  stream.push_back({alphabet, transaction.second});
  return true;
}

bool CoreParser::IsFinal() const {
  return is_valid_path_so_far && ContainsKey(machine->dfa.final_states,
                                             current_state);
}

void CoreParser::ParseOrDie(CoreParseNode* output) {
  if (not Parse(output)) {
    throw Error(Error::PARSING_ERROR_INCOMPLETE_TOKENS)
                .PossibleAlphabets(PossibleAlphabets(10))();
  }
}

bool CoreParser::Parse(CoreParseNode* output, Error* error) {
  if (not Parse(output)) {
    *error = Error(Error::PARSING_ERROR_INCOMPLETE_TOKENS)
                .PossibleAlphabets(PossibleAlphabets(10))();
    return false;
  }
  return true;
}

bool CoreParser::Parse(CoreParseNode* output) {
  if (not IsFinal()) return false;
  enum StreamElement {START_BRANCH, END_BRANCH, ALPHABET};
  vector<pair<int, int>> tracking_stack;
  int ns = machine->dfa.nfa_final_state_map.at(current_state);
  APARSE_DEBUG_ASSERT(ContainsKey(machine->nfa.final_states, ns));
  auto& nfa = machine->nfa;
  vector<AParseMachine::ParsingStream> parsing_stream(1+stream.size());
  parsing_stream[stream.size()] = nfa.states[ns].parsing_stream;
  int pns = 0;  // previous nfa state
  // cout << "DFA path: " << dfa_path << endl;
  // cout << "Reverse NFA Path: " << ns << "," << std::endl;

  for (int i = static_cast<int>(stream.size())-1; i >= 0; i--) {
    auto txn = stream[i];
    int pds = dfa_path[i];
    switch (txn.second.type) {
      case StackOperation::NOOP: {
        pns = machine->back_tracking.map1.at(make_tuple(ns, pds, txn.first));
        parsing_stream[i] = nfa.states[pns].edges.at(txn.first)
                                                  .at(ns).parsing_stream;
        break;
      }
      case StackOperation::POP: {
        pns = machine->back_tracking.map1.at(make_tuple(ns, pds, txn.first));
        tracking_stack.push_back({pns, ns});
        parsing_stream[i] = nfa.states[pns].parsing_stream;
        break;
      }
      case StackOperation::PUSH: {
        auto item = tracking_stack.back();
        tracking_stack.pop_back();
        pns = machine->back_tracking.map2.at(
                      make_tuple(pds, item.first, item.second));
        parsing_stream[i] = machine->back_tracking.jump_parsing_stream.at(
                                    make_tuple(pns, item.first, item.second));
        break;
      }
      default: assert(false);
    }
    ns = pns;
  }
  vector<CoreParseNode*> branching_stack;
  output->start = 0;
  output->end = stream.size();
  branching_stack.push_back(output);
  for (int i=0; i < parsing_stream.size(); i++) {
    for (auto& ps : parsing_stream[i]) {
      if (ps.first == AParseMachine::START_BRANCH) {
        branching_stack.back()->children.push_back(
                                            CoreParseNode({ps.second, i}));
        branching_stack.push_back(&branching_stack.back()->children.back());
      } else {
        branching_stack.back()->end = i;
        branching_stack.pop_back();
      }
    }
  }
  APARSE_DEBUG_ASSERT(branching_stack.size() == 1);
  APARSE_DEBUG_ASSERT(output->children.size() > 0);
  return true;
}


}  // namespace aparse

