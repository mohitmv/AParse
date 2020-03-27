// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/utils.hpp"

namespace aparse {
namespace utils {

bool TopologicalSortingInGraph(const SimpleGraph& graph,
                               vector<int>* topological_sorted,
                               vector<int>* cycle_payh) {
  std::unordered_set<int> ancestors;
  std::unordered_set<int> visited;
  // returns true if there is no cycle.
  std::function<bool(int)> lDFA = [&](int u) {
    ancestors.insert(u);
    for (auto& oe : graph.at(u)) {
      if ((!qk::ContainsKey(visited, oe)) &&
          (qk::ContainsKey(ancestors, oe) || !lDFA(oe))) {
        return false;
      }
    }
    ancestors.erase(u);
    visited.insert(u);
    topological_sorted->push_back(u);
    return true;
  };
  for (auto& item : graph) {
    if (!qk::ContainsKey(visited, item.first) && !lDFA(item.first)) {
      return false;
    }
  }
  return true;
}

bool IsLiteralName(const std::string& s) {
  for (int i = 0; i < s.size(); i++) {
    unsigned char c = s[i];
    if (s[i] == '_' or (i == 0 ? std::isalpha(c) : std::isalnum(c))) {  // NOLINT
      continue;
    }
    return false;
  }
  return true;
}

}  // namespace utils
}  // namespace aparse
