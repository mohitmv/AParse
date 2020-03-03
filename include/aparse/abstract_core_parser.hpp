// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_ABSTRACT_CORE_PARSER_HPP_
#define _APARSE_ABSTRACT_CORE_PARSER_HPP_

#include <string>
#include <utility>
#include <vector>

#include <quick/utility.hpp>
#include <quick/debug_stream_decl.hpp>

#include "aparse/error.hpp"

namespace aparse {

// Node of ParseTree.
struct CoreParseNode {
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
  void DebugStream(qk::DebugStream& ds) const;

  int label = 0;
  int start = 0, end = 0;  // start: inclusive, end: exclusive;
  vector<CoreParseNode> children;
};


class AbstractCoreParser {
 public:
  AbstractCoreParser() = default;
  virtual ~AbstractCoreParser() = default;
  virtual void SetAParseMachine(const qk::AbstractType* machine) = 0;

  virtual bool Parse(CoreParseNode* output) = 0;
  virtual void ParseOrDie(CoreParseNode* output) = 0;
  virtual bool Parse(CoreParseNode* output, Error* error) = 0;
  // virtual bool Parse(const vector<Alphabet>& stream, CoreParseNode* output);
  // virtual void ParseOrDie(const vector<Alphabet>& stream,
  //                         CoreParseNode* output);
  // virtual bool Parse(const vector<Alphabet>& stream,
  //                    CoreParseNode* output,
  //                    Error* error);

  virtual bool Feed(Alphabet alphabet) = 0;
  virtual void FeedOrDie(Alphabet alphabet) = 0;
  virtual bool Feed(Alphabet alphabet, Error* error) = 0;
  virtual bool Feed(const vector<Alphabet>& stream) = 0;
  virtual void FeedOrDie(const vector<Alphabet>& stream) = 0;
  virtual bool Feed(const vector<Alphabet>& stream, Error* error) = 0;

  virtual bool CanFeed(Alphabet alphabet) const = 0;
  virtual bool IsFinal() const = 0;
  virtual void Reset() = 0;
  virtual const vector<Alphabet>& GetStream() const = 0;
  virtual unordered_set<Alphabet> PossibleAlphabets() const = 0;
  // deprecated.
  virtual unordered_set<Alphabet> PossibleAlphabets(int k) const = 0;  // return k only.
};

}  // namespace aparse

#endif  // _APARSE_CORE_PARSER_HPP_

