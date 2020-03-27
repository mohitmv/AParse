// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/simple_aparse_grammar_builder.hpp"

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <sstream>
#include <quick/debug_stream.hpp>

#include <quick/debug.hpp>

namespace aparse {
namespace helpers {
void AParseGrammarIntAssignment(
    const vector<pair<string, RegexBuilderObject>>& string_rules,
    const string& main_non_terminal,
    const vector<pair<string, string>>& branching_alphabets,
    AParseGrammar* grammar,
    std::unordered_map<string, int>* string_map) {
  std::unordered_set<string> symbols;
  std::unordered_set<string> non_terminals;
  for (auto& rule : string_rules) {
    non_terminals.insert(rule.first);
    rule.second.GetSymbols(&symbols);
  }
  non_terminals.insert(main_non_terminal);
  for (auto& item : branching_alphabets) {
    symbols.insert(item.first);
    symbols.insert(item.second);
  }
  auto terminals = qk::SetMinus(symbols, non_terminals);
  int counter = 0;
  for (auto& t : terminals) {
    (*string_map)[t] = counter++;
  }
  grammar->alphabet_size = counter;
  for (auto& nt : non_terminals) {
    (*string_map)[nt] = counter++;
  }
  for (auto& rule : string_rules) {
    grammar->rules.emplace_back(make_pair(string_map->at(rule.first),
                                          rule.second.BuildRegex(*string_map)));
  }
  for (auto& item : branching_alphabets) {
    grammar->branching_alphabets.emplace_back(
        make_pair(string_map->at(item.first),
                  string_map->at(item.second)));
  }
  grammar->main_non_terminal = string_map->at(main_non_terminal);
  for (auto& item : *string_map) {
    grammar->alphabet_map.insert(make_pair(item.second, item.first));
  }
}

}  // namespace helpers

}  // namespace aparse
