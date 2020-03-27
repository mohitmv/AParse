// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_SRC_V1_CORE_PARSER_HPP_
#define _APARSE_SRC_V1_CORE_PARSER_HPP_

#include <string>
#include <utility>
#include <vector>

#include "aparse/aparse_machine.hpp"
#include "aparse/error.hpp"

namespace aparse {

struct CoreParseNode {
  int label = 0;
  int start = 0, end = 0;  // start: inclusive, end: exclusive;
  vector<CoreParseNode> children;
  CoreParseNode() {}
  explicit CoreParseNode(int label): label(label) {}
  CoreParseNode(int label, int start): label(label), start(start) {}
  CoreParseNode(int label, const pair<int, int>& range): label(label),
                                                         start(range.first),
                                                         end(range.second) {}
  CoreParseNode(int label,
                const pair<int, int>& range,
                const vector<CoreParseNode>& children): label(label),
                                                        start(range.first),
                                                        end(range.second),
                                                        children(children) {}
  CoreParseNode(int label,
                const pair<int, int>& range,
                vector<CoreParseNode>&& children)
       : label(label),
         start(range.first),
         end(range.second),
         children(std::move(children)) {}
  bool operator==(const CoreParseNode& rhs) const;
  string DebugString() const;
};


class CoreParser {
 public:
  CoreParser() {}
  explicit CoreParser(const AParseMachine* machine)
    : machine(machine),
      current_state(machine->dfa.start_state) {
    dfa_path.push_back(machine->dfa.start_state);
  }
  void SetAParseMachine(const AParseMachine* machine);

  bool Parse(CoreParseNode* output);
  void ParseOrDie(CoreParseNode* output);
  bool Parse(CoreParseNode* output, Error* error);
  bool Parse(const vector<Alphabet>& stream, CoreParseNode* output);
  void ParseOrDie(const vector<Alphabet>& stream, CoreParseNode* output);
  bool Parse(const vector<Alphabet>& stream,
             CoreParseNode* output,
             Error* error);

  bool Feed(Alphabet alphabet);
  void FeedOrDie(Alphabet alphabet);
  bool Feed(Alphabet alphabet, Error* error);
  bool Feed(const vector<Alphabet>& stream);
  void FeedOrDie(const vector<Alphabet>& stream);
  bool Feed(const vector<Alphabet>& stream, Error* error);

  bool CanFeed(Alphabet alphabet) const;
  bool IsFinal() const;
  void Reset();
  Alphabet GetAlphabet(int index) const;
  vector<Alphabet> PossibleAlphabets() const;
  vector<Alphabet> PossibleAlphabets(int k) const;  // return k only.
  string DebugString() const;

 private:
  using StackOperation = AParseMachine::StackOperation;
  using Transaction = pair<Alphabet, StackOperation>;
  void ConstructNFAPath();
  // bool ParseInternal(CoreParseNode* output);
  // bool FeedInternal(Alphabet alphabet);

  const AParseMachine* machine = nullptr;
  // Invariant: Update the default values of these members in Reset method.
  bool is_valid_path_so_far = true;
  int current_state;
  vector<int> stack;
  vector<int> dfa_path;  // List of DFA states in path (excluding current_state)
  vector<Transaction> stream;
};

}  // namespace aparse

#endif  // _APARSE_SRC_V1_CORE_PARSER_HPP_
