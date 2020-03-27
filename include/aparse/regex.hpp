// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_REGEX_HPP_
#define APARSE_REGEX_HPP_

#include <vector>
#include <utility>
#include <string>
#include <unordered_map>

#include "aparse/utils/very_common_headers.hpp"
#include "quick/debug_stream_decl.hpp"

namespace aparse {

/** Regex is a simple RegularExpression object.
 *  Learn more about the definition of Regex at aparse.readthedocs.io */
struct Regex {
  enum RegexType {EPSILON, ATOMIC, UNION, CONCAT, KPLUS, KSTAR, ID};
  Regex() {}
  explicit Regex(RegexType regex_type): type(regex_type) {}  // NOLINT
  explicit Regex(Alphabet alphabet): type(ATOMIC), alphabet(alphabet) {}  // NOLINT
  Regex(RegexType regex_type, const std::vector<Regex>& children)
       : type(regex_type),
         children(children) {}
  Regex(RegexType regex_type, std::vector<Regex>&& children)
     : type(regex_type),
       children(std::move(children)) {}
  bool HasChildren() const;
  /** String representation of RegexType enum */
  static std::string RegexTypeString(RegexType t);
  Regex& operator+(const Regex& other);
  Regex& operator+(Regex&& other);
  Regex& operator|(const Regex& other);
  bool operator==(const Regex& rhs) const;
  Regex& SetLabel(int label);
  std::string DebugString() const;
  // @alphabet_map contains string-meaning of int-alphabets.
  std::string DebugString(
      const std::unordered_map<int, std::string>& alphabet_map) const;
  void DebugStream(qk::DebugStream&) const;
  std::size_t GetHash() const;
  void Clear();

  RegexType type = EPSILON;
  /** used only when children.size() == 0 */
  Alphabet alphabet = 0;
  std::vector<Regex> children;
  /** optional field for external identification purpose. */
  int label = 0;
};
}  // namespace aparse

#endif  // APARSE_REGEX_HPP_
