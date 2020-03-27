// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/internal_lexer_builder.hpp"

#include <memory>
#include <unordered_map>

#include "src/lexer_machine_builder.hpp"

namespace aparse {
// sttaic
bool InternalLexerBuilder::Build(const InternalLexerGrammar& lexer_grammar,
                                 Lexer* output) {
  if (lexer_grammar.rules.size() == 0) {
    throw Error(Error::LEXER_BUILDER_ERROR_MUST_HAVE_NON_ZERO_RULES)();
  }
  int index_counter = 1;
  unordered_map<int, LexerMachine::DFA> dfa_map;
  vector<int> section_id_list;
  for (auto& section : lexer_grammar.rules) {
    Regex regex_union(Regex::UNION);
    for (auto& rule : section.second) {
      auto regex = rule.regex;
      regex.label = index_counter++;
      output->pattern_actions[regex.label] = rule.action;
      regex_union.children.emplace_back(regex);
    }
    auto nfa = LexerMachineBuilder::BuildNFA(regex_union);
    LexerMachineBuilder::BuildDFA(nfa, &dfa_map[section.first]);
  }
  output->main_section = lexer_grammar.main_section;
  LexerMachineBuilder::MergeDFA(
    dfa_map,
    lexer_grammar.main_section,
    &output->machine.dfa,
    &output->section_to_start_state_mapping);
  output->machine.initialized = true;
  return output->Finalize();
}


}  // namespace aparse
