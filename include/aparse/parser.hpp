// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_PARSER_HPP_
#define _APARSE_PARSER_HPP_

#include <tuple>
#include <memory>
#include <vector>
#include <unordered_set>

#include "quick/hash.hpp"
#include "quick/stl_utils.hpp"

#include "aparse/common_headers.hpp"
#include "aparse/error.hpp"
#include "aparse/abstract_core_parser.hpp"
#include "aparse/parser_scope.hpp"
#include "aparse/syntax_tree_maker.hpp"

#include <quick/debug.hpp>
#include <quick/utility.hpp>

namespace aparse {

class Parser;

class ParserInstance {
 public:
  ParserInstance() = default;
  explicit ParserInstance(const Parser& parser);

  void Init(const Parser& parser);
  void Reset();
  bool Feed(Alphabet a);
  bool Feed(Alphabet a, Error* error);
  void FeedOrDie(Alphabet a);
  bool Feed(const vector<Alphabet>& s);
  bool Feed(const vector<Alphabet>& s, Error* error);
  void FeedOrDie(const vector<Alphabet>& s);
  bool End();
  bool End(Error* error);
  void EndOrDie();
  bool IsFinal() const;
  unordered_set<Alphabet> PossibleAlphabets(int k) const;
  unordered_set<Alphabet> PossibleAlphabets() const;

  // Assumes .End() succeed.
  template<typename SyntaxTreeNode>
  void CreateSyntaxTree(SyntaxTreeNode* output) {
    ParserScopeBase<SyntaxTreeNode> scope;
    CreateSyntaxTree(&scope, output);
  }

  // Assumes .End() succeed.
  // ToDo(Mohit): Validate if `core_parser` is initilized or not.
  template<typename SyntaxTreeNode, typename ParserScope>
  void CreateSyntaxTree(ParserScope* scope, SyntaxTreeNode* output) {
    syntax_tree_maker->Build(parse_tree,
                             core_parser->GetStream(),
                             scope,
                             output);
  }

  // Replace this shared_ptr by container_ptr.
  std::shared_ptr<AbstractCoreParser> core_parser;
  CoreParseNode parse_tree;
  const SyntaxTreeMaker* syntax_tree_maker;
};


class Parser : public quick::AbstractType {
 public:
  // More the compression -> smaller the size of AParseMachine -> smaller the
  //    AParseMachine build time -> lower the expressibility ->
  //    higher the run time latency (Parsing).
  enum AParseMachineType : uint8_t {VERY_LOW_COMPRESSION,
                                    VERY_HIGH_COMPRESSION};

  ParserInstance CreateInstance() const {
    return ParserInstance(*this);
  }
  bool Finalize();
  bool IsFinalized () const { return is_finalized;}

  std::unique_ptr<qk::AbstractType> machine;
  std::unique_ptr<SyntaxTreeMaker> syntax_tree_maker;
  std::vector<utils::any> rule_actions;
  std::vector<vector<Alphabet>> rule_atoms;
  std::vector<int> rule_non_terminals;
  AParseMachineType machine_type = AParseMachineType::VERY_HIGH_COMPRESSION;
  bool is_finalized = false;
};


}  // namespace aparse

#endif  // _APARSE_PARSER_HPP_
