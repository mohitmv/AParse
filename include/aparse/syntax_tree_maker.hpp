// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_SYNTAX_TREE_MAKER_HPP_
#define APARSE_SYNTAX_TREE_MAKER_HPP_

#include <tuple>
#include <memory>
#include <utility>
#include <vector>

#include "quick/stl_utils.hpp"

#include "aparse/common_headers.hpp"
#include "aparse/error.hpp"
#include "aparse/parser_scope.hpp"
#include "aparse/utils/any.hpp"

#include <quick/debug.hpp>
#include <quick/utility.hpp>

namespace aparse {

/** Create the SyntaxTree from the given ParseTree.
    It use the client defined RuleActions. Learn more about RuleActions at
    https://aparse.readthedocs.io */
struct SyntaxTreeMaker {
  SyntaxTreeMaker(const std::vector<utils::any>& rule_actions,
                  const std::vector<vector<Alphabet>>& rule_atoms,
                  const std::vector<int>& rule_non_terminals)
  : rule_actions(rule_actions),
    rule_atoms(rule_atoms),
    rule_non_terminals(rule_non_terminals) {}

  template<typename ParserScope, typename SyntaxTreeNode>
  void Build(const CoreParseNode& parse_tree,
             const vector<Alphabet>& stream,
             ParserScope* parsing_scope,
             SyntaxTreeNode* output) const {
    APARSE_ASSERT(rule_actions.size() > 0);
    APARSE_ASSERT(rule_actions.size() == rule_atoms.size());
    auto& tbc = *parsing_scope->MutableTreeBuildingConstructs();
    std::function<void(const CoreParseNode& node,
                       SyntaxTreeNode* output)> lCreateST;
    lCreateST = [&](const CoreParseNode& node, SyntaxTreeNode* output) {
      vector<SyntaxTreeNode> child_st_list;
      child_st_list.resize(node.children.size());
      for (int s = node.start, c_index = 0; s < node.end;) {
        if (c_index < node.children.size() &&
             s == node.children[c_index].start) {
          lCreateST(node.children[c_index], &child_st_list[c_index]);
          s = node.children[c_index].end;
          c_index++;
        } else {
          s++;
        }
      }
      tbc.Clear();
      tbc.child_st_list_2 = std::move(child_st_list);
      for (int s = node.start, c_index = 0; s < node.end;) {
        if (c_index < node.children.size() &&
             s == node.children[c_index].start) {
          auto& child = node.children[c_index];
          int nt = rule_non_terminals.at(child.label);
          tbc.child_st_list_1[nt].push_back(c_index);
          tbc.is_terminal[nt] = false;
          tbc.range_list_1[nt].push_back(make_pair(child.start,
                                                             child.end));
          tbc.range_list_2.push_back(make_pair(child.start, child.end));
          s = node.children[c_index].end;
          c_index++;
        } else {
          Alphabet a = stream.at(s);
          tbc.is_terminal[a] = true;
          tbc.token_stream_index_map[a].push_back(s);
          tbc.token_stream_index_map_2.push_back(s);
          tbc.token_list_1.insert(a);
          tbc.token_list_2.push_back(a);
          s++;
        }
      }
      tbc.rule_atoms_ = &rule_atoms.at(node.label);
      tbc.range = make_pair(node.start, node.end);
      auto& rule_action = rule_actions.at(node.label);
      using RuleActionType = std::function<void(ParserScope*, SyntaxTreeNode*)>;
      if (rule_action.has_value()) {
        if (rule_action.can_cast_to<RuleActionType>()) {
          rule_action.cast_to<RuleActionType>()(parsing_scope, output);
        } else {
          throw Error(Error::INVALID_RULE_ACTION_TYPE);
        }
      }
    };
    lCreateST(parse_tree.children[0], output);
  }

 private:
  const std::vector<utils::any>& rule_actions;
  const std::vector<vector<Alphabet>>& rule_atoms;
  const std::vector<int>& rule_non_terminals;
};

}  // namespace aparse

#endif  // APARSE_SYNTAX_TREE_MAKER_HPP_
