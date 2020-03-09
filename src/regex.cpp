// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/regex.hpp"

#include <sstream>
#include <functional>

#include "quick/stl_utils.hpp"
#include "quick/hash.hpp"
#include "quick/debug_stream.hpp"
#include "src/regex_helpers.hpp"
#include "src/helpers.hpp"

namespace aparse {
using std::unordered_set;
using std::vector;

void Regex::Clear() {
  *this = Regex();
}

// static
std::string Regex::RegexTypeString(RegexType t) {
  switch (t) {
    case EPSILON:
      return "EPSILON";
    case ATOMIC:
      return "ATOMIC";
    case UNION:
      return "UNION";
    case CONCAT:
      return "CONCAT";
    case KPLUS:
      return "KPLUS";
    case KSTAR:
      return "KSTAR";
    case ID:
      return "ID";
  }
  assert(false);
  return "";
}


std::size_t Regex::GetHash() const {
  return quick::HashFunction(make_tuple(type,
                                        label,
                                        quick::HashFunction(children)));
}

bool Regex::HasChildren() const {
  switch (type) {
    case ATOMIC:
    case EPSILON:
      return false;
    default:
      return true;
  }
}

bool Regex::operator==(const Regex& rhs) const {
  return (label == rhs.label &&
           type == rhs.type &&
           alphabet == rhs.alphabet &&
           children == rhs.children);
}

void Regex::DebugStream(qk::DebugStream& ds) const {
  ds << DebugString();
}

string Regex::DebugString() const {
  unordered_map<int, string> alphabet_map;
  return DebugString(alphabet_map);
}

string Regex::DebugString(
    const unordered_map<int, string>& alphabet_map) const {
  std::ostringstream oss;
  bool print_symbol = (alphabet_map.size() > 0);
  std::function<void(const Regex&)> lPrintRegex = [&](const Regex& input) {
    if (input.label > 0) {
      oss << "(";
    }
    switch (input.type) {
      case EPSILON:
        oss << "EPSILON"; break;
      case ATOMIC: {
        if (print_symbol && qk::ContainsKey(alphabet_map, input.alphabet)) {
          oss << helpers::GetAlphabetString(alphabet_map.at(input.alphabet));
        } else {
          oss << input.alphabet;
        }
        break;
      }
      case UNION: {
        if (input.children.size() <= 1) {
          oss << "U";
        }
        oss << "(";
        for (int i = 0; i < input.children.size(); i++) {
          lPrintRegex(input.children[i]);
          if (i+1 < input.children.size()) {  // Assert(input.children.size()>0)
            oss << " | ";
          }
        }
        oss << ")";
        break;
      }
      case CONCAT: {
        if (input.children.size() <= 1) {
          oss << "C(";
        } else {
          oss << "(";
        }
        for (int i = 0; i < input.children.size(); i++) {
          lPrintRegex(input.children[i]);
          if (i+1 < input.children.size()) {
            oss << " ";
          }
        }
        oss << ")";
        break;
      }
      case KSTAR: {
        oss << "(";
        lPrintRegex(input.children[0]);
        oss << ")*";
        break;
      }
      case KPLUS: {
        oss << "(";
        lPrintRegex(input.children[0]);
        oss << ")+";
        break;
      }
      case ID: {
        oss << "(";
        lPrintRegex(input.children[0]);
        oss << ")";
        break;
      }
      default:
        APARSE_DEBUG_ASSERT(false, (int)input.type);
        break;
    }
    if (input.label > 0) {
      oss << ")[" << input.label << "]";
    }
  };
  lPrintRegex(*this);
  return oss.str();
}

Regex& Regex::operator+(const Regex& other) {
  if (type == CONCAT) {
    if (other.type == CONCAT) {
      qk::InsertToVector(other.children, &children);
    } else {
      children.push_back(other);
    }
  } else {
    *this = Regex(Regex::CONCAT, {std::move(*this), other});
  }
  return *this;
}

Regex& Regex::operator+(Regex&& other) {
  if (type == CONCAT) {
    if (other.type == CONCAT) {
      qk::InsertToVectorMoving(other.children, &children);
    } else {
      children.emplace_back(other);
    }
  } else {
    *this = Regex(Regex::CONCAT, {std::move(*this), std::move(other)});
  }
  return *this;
}

Regex& Regex::SetLabel(int label) {
  this->label = label;
  return *this;
}

}  // namespace aparse
