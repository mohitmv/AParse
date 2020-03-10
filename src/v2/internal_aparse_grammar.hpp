// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_SRC_V2_INTERNAL_APARSE_GRAMMAR_HPP_
#define APARSE_SRC_V2_INTERNAL_APARSE_GRAMMAR_HPP_

#include <utility>
#include <list>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>

#include "quick/unordered_map.hpp"
#include "quick/debug_stream_decl.hpp"

#include "aparse/common_headers.hpp"
#include "aparse/aparse_grammar.hpp"

namespace aparse {
namespace v2 {

/** InternalAParseGrammar' is a preprocessed version of AparseGrammar. We offer
 *  a lot of flexibility in AParseGrammar to make it easy to design. However
 *  these flexibilities can be reduced to non-flexible version of AParseGrammar
 *  by doing some preprocessing.
 *  Eg:
 *  1). We allow a single non-terminal to be defined in multiple rules. After
 *      preprocessing step, these non-terminals are combibed into single rule by
 *      joining the corrosponding expressions with UNION operator.
 *  2). In addition to that we attach a label (a unique-id (integer)) on the
 *      rule-regex so that later we can identify the grammar-rule a regex
 *      corrosponds to.
 *  3). Constructs the enclosed-non-terminals by extraing out the
 *      sub-expressions wrapped in branching alphabets.
 *
 * Usage: InternalAParseGrammar igrammar; igrammar.Build(...); */
class InternalAParseGrammar {
 public:
  InternalAParseGrammar() {}
  // Not idempotent.
  void Init(const AParseGrammar& grammar);
  void DebugStream(qk::DebugStream&) const;

  ////////////////////// Helpers Functions For Alphabets ////////////////////
  //
  // Both `IsBAStart` and `IsBAEnd` assumes that `ba_map` and `ba_inverse_map`
  // is populated before calling them. Avoiding necessary fatal-assertions here.
  //
  // Checks if a given regex is a atomic regex and it's alphabet is
  // start-branching-alphabet.
  bool IsBAStart(const Regex& r) const;
  // Checks if a given regex is a atomic regex and it's alphabet is
  // end-branching-alphabet.
  bool IsBAEnd(const Regex& r) const;
  // Checks if a given regex is a atomic regex and it's alphabet is
  // end-branching-alphabet and also it's closing end of @start_ba.
  bool IsBAEnd(const Regex& r, Alphabet start_ba) const;
  //
  /////////////////// ----x----x----x----x---- ///////////////////////////////



 protected:
  // @id_counter: enclosed_non_terminal_id_counter
  void ConstructEnclosedNonTerminals(
      std::vector<std::pair<int, Regex>>* rules_list,
      int* id_counter);

  // Regex main_regex;
  // Mapping from enclosed-non-terminals to corrosponding Regex.
  // std::unordered_map<EnclosedsAlphabet, SubRegex> sub_regex_map;

 public:
  int alphabet_size;
  int main_non_terminal;
  std::unordered_map<int, Regex> rules;
  // map(enclosed_non_terminal -> (branch_start_alphabet, regex))
  std::unordered_map<int, Regex> enclosed_rules;
  std::unordered_map<int, Alphabet> enclosed_ba_alphabet;
  std::unordered_map<int, int> regex_label_to_original_rule_number_mapping;
  // branching-alphabets-map.
  std::unordered_map<int, int> ba_map;
  // map(value -> key) of `ba_map`.
  std::unordered_map<int, int> ba_inverse_map;
  // Map(every non-terminal -> list of other non-terminals it's dependent on)
  std::unordered_map<int, std::unordered_set<int>> dependency_graph;
  std::vector<int> topological_sorted_non_terminals;
  std::unordered_set<int> non_terminals;
  std::unordered_set<int> enclosed_non_terminals;
};

}  // namespace v2
}  // namespace aparse

#endif  // APARSE_SRC_V2_INTERNAL_APARSE_GRAMMAR_HPP_
