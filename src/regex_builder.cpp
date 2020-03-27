// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/regex_builder.hpp"

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <sstream>
#include <set>

#include <quick/debug_stream.hpp>
#include <quick/stl_utils.hpp>

#include <quick/debug.hpp>

namespace aparse {

using StringMap = std::unordered_map<std::string, int>;
using AlphabetMap = std::unordered_map<int, std::string>;


RegexBuilderObject& RegexBuilderObject::operator+(
      const RegexBuilderObject& other) {
  if (type == Regex::CONCAT) {
    children.emplace_back(other);
  } else {
    std::vector<RegexBuilderObject> new_children;
    new_children.emplace_back(std::move(*this));
    new_children.emplace_back(other);
    *this = RegexBuilderObject(Regex::CONCAT);
    this->children = std::move(new_children);
  }
  return *this;
}

RegexBuilderObject& RegexBuilderObject::operator|(
      const RegexBuilderObject& other) {
  if (type == Regex::UNION) {
    children.emplace_back(other);
  } else {
    std::vector<RegexBuilderObject> new_children;
    new_children.emplace_back(std::move(*this));
    new_children.emplace_back(other);
    *this = RegexBuilderObject(Regex::UNION);
    this->children = std::move(new_children);
  }
  return *this;
}

RegexBuilderObject& RegexBuilderObject::Kplus() {
  vector<RegexBuilderObject> new_children;
  new_children.emplace_back(std::move(*this));
  this->type = Regex::KPLUS;
  this->children = std::move(new_children);
  return *this;
}


RegexBuilderObject& RegexBuilderObject::Optional() {
  vector<RegexBuilderObject> new_children;
  new_children.emplace_back(RegexBuilderObject(Regex::EPSILON));
  new_children.emplace_back(std::move(*this));
  this->type = Regex::UNION;
  this->children = std::move(new_children);
  return *this;
}

RegexBuilderObject& RegexBuilderObject::Kstar() {
  vector<RegexBuilderObject> new_children;
  new_children.emplace_back(std::move(*this));
  this->type = Regex::KSTAR;
  this->children = std::move(new_children);
  return *this;
}

void RegexBuilderObject::GetSymbols(
    std::unordered_set<std::string>* output) const {
  switch (type) {
    case Regex::EPSILON: break;
    case Regex::ATOMIC: {
      if (not alphabet_string.empty()) {
        output->insert(alphabet_string);
      }
      break;
    }
    case Regex::CONCAT:
    case Regex::UNION:
    case Regex::KSTAR:
    case Regex::KPLUS: {
    case Regex::ID:
      for (auto& c : children) {
        c.GetSymbols(output);
      }
      break;
    }
    default: assert(false);
  }
}

std::unordered_set<std::string> RegexBuilderObject::GetSymbols() const {
  std::unordered_set<std::string> output;
  GetSymbols(&output);
  return output;
}

Regex RegexBuilderObject::BuildRegex(const StringMap& allowed_symbols) const {
  Regex output;
  BuildRegex(allowed_symbols, &output);
  return output;
}

void RegexBuilderObject::BuildRegex(const StringMap& allowed_symbols,
                                    Regex* output) const {
  output->label = label;
  output->type = type;
  switch (type) {
    case Regex::EPSILON: break;
    case Regex::ATOMIC: {
      if (not alphabet_string.empty()) {
        if (qk::ContainsKey(allowed_symbols, alphabet_string)) {
          output->alphabet = allowed_symbols.at(alphabet_string);
        } else {
          throw std::runtime_error(string("Symbol ") + alphabet_string +
                                   " not found");
        }
      } else {
        output->alphabet = alphabet;
      }
      break;
    }
    case Regex::CONCAT:
    case Regex::UNION:
    case Regex::KSTAR:
    case Regex::KPLUS:
    case Regex::ID: {
      output->children.resize(children.size());
      for (int i = 0; i < children.size(); i++) {
        children[i].BuildRegex(allowed_symbols, &output->children[i]);
      }
      break;
    }
    default: assert(false);
  }
}


RegexBuilderObject& RegexBuilderObject::SetLabel(int label) {
  this->label = label;
  return *this;
}

void RegexBuilderObject::DebugStream(qk::DebugStream& ds) const {
  if (label > 0) {
    ds << "label = " << label << "\n";
  }
  ds << "type = " << Regex::RegexTypeString(type);
  switch (type) {
    case Regex::EPSILON:
      break;
    case Regex::ATOMIC: {
      ds << "\n" << "atom = ";
      if (alphabet_string.empty()) {
        ds << alphabet;
      } else {
        ds << alphabet_string;
      }
      break;
    }
    case Regex::UNION:
    case Regex::CONCAT:
    case Regex::KPLUS:
    case Regex::ID:
    case Regex::KSTAR:
      ds << "\n" << "children = " << children;
      break;
    default: assert(false);
  }
}

Regex RegexBuilderObject::BuildRegex() const {
  AlphabetMap alphabet_map;
  return BuildRegex(&alphabet_map);
}

Regex RegexBuilderObject::BuildRegex(AlphabetMap* alphabet_map) const {
  APARSE_DEBUG_ASSERT(alphabet_map->size() == 0);
  std::set<string> symbols;
  qk::InsertToSet(GetSymbols(), &symbols);
  StringMap symbols_map;
  for (auto& s : symbols) {
    int len = symbols_map.size();
    symbols_map.insert(make_pair(s, len));
    alphabet_map->insert(make_pair(len, s));
  }
  return BuildRegex(symbols_map);
}

// ToDo(Mohit): Enable this method with updated signature.

// Regex RegexBuilder::BuildRegex(const RegexBuilderObject& rbo) {
//   unordered_set<int> alphabet_set;
//   qk::GetValues(alphabet_map, &alphabet_set);
//   int counter = 0;
//   auto lAssignNumber = [&](const std::string& s) {
//     if (qk::ContainsKey(alphabet_map, s)) {
//       return;
//     }
//     while (qk::ContainsKey(alphabet_set, counter)) {
//       counter++;
//     }
//     alphabet_set.insert(counter);
//     alphabet_map[s] = counter;
//   };
//   auto symbols =  rbo.GetSymbols();
//   for (auto& s : symbols) {
//     lAssignNumber(s);
//   }
//   return rbo.BuildRegex(alphabet_map);
// }

}  // namespace aparse
