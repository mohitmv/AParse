// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_SRC_UTILS_HPP_
#define APARSE_SRC_UTILS_HPP_

#include <functional>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>

#include <quick/stl_utils.hpp>

#include "aparse/common_headers.hpp"

namespace aparse {
using quick::ContainsKey;
namespace utils {

/** map(node_id -> set of adjacent nodes) */
using SimpleGraph = std::unordered_map<int, std::unordered_set<int>>;

/** returns true if no cycle is found. In case of success (i.e. no-cycle),
 *  @topological_sorted will be set appropriately o.w. @cycle_path will be set.
 */
bool TopologicalSortingInGraph(const SimpleGraph& graph,
                               vector<int>* topological_sorted,
                               vector<int>* cycle_path);


// ToDo(Mohit): This is ridiculous design. Improve it !
template<typename NodeType, typename LChildren, typename LValue>
string PrintPrettyTree(const NodeType* node,
                       LChildren& children,  // NOLINT
                       LValue& value) {  // NOLINT
  std::ostringstream oss;
  int hspace = 5;
  int vspace = 3;
  auto lRepeatedString = [&](string a, int times) {
    string output = "";
    for (int i = 0; i < times; i++) {
      output += a;
    }
    return output;
  };
  std::function<void(const NodeType*, int)> lPrint =
                                [&](const NodeType* node, int depth) {
    for (int j=0; j < vspace; j++) {
      for (int i=0; i < depth; i++) {
        oss << lRepeatedString(" ", i == 0 ? 2 : hspace) << "|";
      }
      if (j == vspace-1) {
        oss << lRepeatedString("-", (depth == 0 ? 2 : hspace) + 0)
            << value(node);
      }
      oss << "\n";
    }
    for (auto& child : children(node)) {
      lPrint(child, depth + 1);
    }
  };
  lPrint(node, 0);
  oss << lRepeatedString("\n", 3) << "\n";
  return oss.str();
}

/** Check if the given string follows `[a-zA-Z_][a-zA-Z0-9_]*` regex. */
bool IsLiteralName(const std::string& s);

}  // namespace utils
}  // namespace aparse

#endif  // APARSE_SRC_UTILS_HPP_
