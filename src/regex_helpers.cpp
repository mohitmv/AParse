// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/regex.hpp"

#include <sstream>
#include <functional>

#include "quick/stl_utils.hpp"
#include "quick/hash.hpp"
#include "quick/debug_stream.hpp"
#include "src/regex_helpers.hpp"

namespace aparse {
using std::unordered_set;
using std::vector;

namespace helpers {

bool IsAllRegexChildrenAtomic(const Regex& regex) {
  for (auto& child : regex.children) {
    if (child.type != Regex::ATOMIC) {
      return false;
    }
  }
  return true;
}


unordered_set<Alphabet> GetRegexAtoms(const Regex& r) {
  unordered_set<Alphabet> output;
  std::function<void(const Regex& r)> lGetAtoms = [&](const Regex& r) {
    if (r.HasChildren()) {
      for (const auto& child : r.children) {
        lGetAtoms(child);
      }
    } else if (r.type == Regex::ATOMIC) {
      output.insert(r.alphabet);
    }
  };
  lGetAtoms(r);
  return output;
}

Regex RangeRegex(Alphabet a, Alphabet b) {
  APARSE_ASSERT(a < b);
  Regex output(Regex::UNION);
  for (int i = a; i < b; i++) {
    output.children.emplace_back(i);
  }
  return output;
}

Regex AllAlphabetExceptSomeRegex(const vector<Alphabet>& input,
                                             int alphabet_size) {
  unordered_set<Alphabet> ss(input.begin(), input.end());
  Regex output(Regex::UNION);
  for (int i = 0; i < alphabet_size; i++) {
    if (not qk::ContainsKey(ss, i)) {
      output.children.emplace_back(i);
    }
  }
  APARSE_ASSERT(output.children.size() >= 2);
  return output;
}

Regex SU(const string& s) {
  Regex output(Regex::UNION);
  for (auto c : s) {
    output.children.emplace_back(Regex(static_cast<unsigned char>(c)));
  }
  return output;
}

Regex SC(const string& s) {
  if (s.size() == 1) {
    return A(s[0]);
  } else {
    Regex output(Regex::CONCAT);
    for (auto c : s) {
      output.children.emplace_back(static_cast<unsigned char>(c));
    }
    return output;
  }
}

}  // namespace helpers
}  // namespace aparse
