// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/core_parse_node.hpp"

#include <sstream>

#include "aparse/error.hpp"
#include "quick/debug_stream.hpp"
#include "quick/stl_utils.hpp"

#include "src/utils.hpp"

namespace aparse {

bool CoreParseNode::operator==(const CoreParseNode& rhs) const {
  return (label == rhs.label &&
            start == rhs.start &&
            end == rhs.end &&
            children == rhs.children);
}

void CoreParseNode::DebugStream(qk::DebugStream& ds) const {
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
  ds << utils::PrintPrettyTree(this, lChildren, lValue);
}

bool CoreParseNode::IsInitialized() const {
  return (children.size() > 0);
}

}  // namespace aparse
