// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_PARSER_SCOPE_HPP_
#define APARSE_PARSER_SCOPE_HPP_

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>

#include "quick/stl_utils.hpp"

#include "aparse/common_headers.hpp"
#include "aparse/error.hpp"

#include <quick/utility.hpp>

namespace aparse {

/** ParserScope is used for defining RuleActions, which are used while
 *  constructing SyntaxTree from the ParseTree.
 *  Client may inherit from this class and create a custom ParserScope to be
 *  used in their RuleAction.
 *  Learn more about AParse's ParserScope at https://aparse.readthedocs.io  */
template<typename SyntaxTreeNode>
class ParserScopeBase {
  struct TreeBuildingConstructs {
    const vector<Alphabet>* rule_atoms_;
    unordered_set<Alphabet> token_list_1;
    vector<Alphabet> token_list_2;
    // index in `child_st_list_2` vector.
    unordered_map<int, vector<int>> child_st_list_1;
    unordered_map<int, vector<SyntaxTreeNode>> child_st_list_1_tmp;
    vector<SyntaxTreeNode> child_st_list_2;
    unordered_map<int, vector<pair<int, int>>> range_list_1;
    unordered_map<int, vector<int>> token_stream_index_map;
    vector<int> token_stream_index_map_2;
    vector<pair<int, int>> range_list_2;
    unordered_map<int, bool> is_terminal;
    pair<int, int> range;
    void Clear() {
      child_st_list_2.clear();
      child_st_list_1.clear();
      is_terminal.clear();
      range_list_1.clear();
      range_list_2.clear();
      token_list_1.clear();
      token_list_2.clear();
      token_stream_index_map.clear();
      token_stream_index_map_2.clear();
    }
  };

 public:
  Alphabet GetAlphabet() {
    return tree_building_constructs->token_list_2.at(0);
  }
  vector<int> AlphabetIndexList(int index) {
    auto& tbc = *tree_building_constructs;
    return tbc.token_stream_index_map[tbc.rule_atoms_->at(index)];
  }
  int AlphabetIndex(int index) {
    return AlphabetIndexList(index)[0];
  }
  vector<int> AlphabetIndexList() {
    auto& tbc = *tree_building_constructs;
    return tbc.token_stream_index_map_2;
  }
  int AlphabetIndex() {
    return AlphabetIndexList()[0];
  }
  vector<Alphabet>& GetAlphabetList() {
    return tree_building_constructs->token_list_2;
  }
  vector<SyntaxTreeNode>& ValueList() {
    return tree_building_constructs->child_st_list_2;
  }
  SyntaxTreeNode& Value() {
    auto& tbc = *tree_building_constructs;
    return tbc.child_st_list_2[0];
  }
  SyntaxTreeNode& Value(int index) {
    auto& tbc = *tree_building_constructs;
    int nt = tbc.rule_atoms_->at(index);
    return tbc.child_st_list_2[tbc.child_st_list_1[nt][0]];
  }
  vector<SyntaxTreeNode>& ValueList(int index) {
    auto& tbc = *tree_building_constructs;
    int nt = tbc.rule_atoms_->at(index);
    if (tbc.child_st_list_1[nt].size() == tbc.child_st_list_2.size()) {
      return tbc.child_st_list_2;
    }
    auto& src = tbc.child_st_list_1[nt];
    auto& dst = tbc.child_st_list_1_tmp[nt];
    dst.resize(src.size());
    for (int i = 0; i < src.size(); i++) {
      dst[i] = std::move(tbc.child_st_list_2[src[i]]);
    }
    return dst;
  }
  bool Exists(int index) {
    auto& tbc = *tree_building_constructs;
    int term_id = tbc.rule_atoms_->at(index);
    return qk::ContainsKey(tbc.token_list_1, term_id) ||
            qk::ContainsKey(tbc.child_st_list_1, term_id);
  }
  pair<int, int> Range() {
    return tree_building_constructs->range;
  }
  TreeBuildingConstructs* MutableTreeBuildingConstructs() {
    if (tree_building_constructs == nullptr) {
      tree_building_constructs.reset(new TreeBuildingConstructs());
    }
    return tree_building_constructs.get();
  }
  std::unique_ptr<TreeBuildingConstructs> tree_building_constructs;
};

}  // namespace aparse

#endif  // APARSE_PARSER_SCOPE_HPP_
