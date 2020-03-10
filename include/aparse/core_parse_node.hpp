// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_CORE_PARSE_NODE_HPP_
#define APARSE_CORE_PARSE_NODE_HPP_

#include <string>
#include <utility>
#include <vector>
#include <unordered_set>

#include <quick/utility.hpp>
#include <quick/debug_stream_decl.hpp>

#include "aparse/error.hpp"

namespace aparse {

/** Node of ParseTree. Learn more about the structure at
 *  `https://aparse.readthedocs.io/`. 
 *  Consider this sample grammar G {
 *    Main ::= B  | C
 *    B    ::= b* | c* | C* (ab)+
 *    C    ::= ac
 *  }
 *  Language(G) = {EPSILON, c, cc, ccc, b, bb, bbb, ab, abab, acab, acacab,
 *                 acacacab, ...}
 *  ParseTree(acacacab) = 
 *
 *                           Main
 *                            |
 *                     _ _ _ _B _ _
 *                   _/    _/ | \  \_
 *                  /     /   |  \   \
 *               _ C    _C    C   a   b
 *              /  |   / |   / \
 *             a   c  a  c  a   c
 *
 *  Where a non-leaf node represents a non-terminal. CoreParseNode::label is
 *  the index of rule corrosponding to the non-terminal.
 *  start and end index is the range of substring represented by that node. */
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
  void DebugStream(qk::DebugStream& ds) const;  // NOLINT
  bool Validate() const;
  bool IsInitialized() const;

  /** Currently this label represents the index of matching rule in
   *  AParseGrammar. Rule are numbered 0, 1, 2,... ...(num_rules - 1) */
  int label = 0;
  int start = 0, end = 0;  // start: inclusive, end: exclusive;
  vector<CoreParseNode> children;
};

}  // namespace aparse

#endif  // APARSE_CORE_PARSE_NODE_HPP_
