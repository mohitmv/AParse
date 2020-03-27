// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <iomanip>

#include "aparse/parser_builder.hpp"

#include "src/internal_parser_builder.hpp"
#include "src/parse_regex_rule.hpp"

namespace aparse {
namespace helpers {

uint64_t SimpleChecksum(const std::string& input) {
  uint64_t h = 14695981039346656037U;
  for (int i = 0; i < input.size(); i++) {
    h = static_cast<unsigned char>(input[i]) ^ (h * 1099511628211U);
  }
  return h;
}


uint64_t AdvanceParserRulesToGrammarHash(const ParserGrammar& grammar) {
  std::ostringstream oss;
  oss << std::hex << std::setfill('0') << std::setw(4);
  oss << grammar.rules.size();
  for (const auto& rule : grammar.rules) {
    oss << rule.rule_string.size() << rule.rule_string;
  }
  oss << grammar.string_to_alphabet_map.size();
  for (auto& item : grammar.string_to_alphabet_map) {
    oss << item.first.size() << item.first << item.second;
  }
  oss << grammar.branching_alphabets.size();
  for (auto& item : grammar.branching_alphabets) {
    oss << item.first << item.second;
  }
  oss << grammar.main_non_terminal.size() << grammar.main_non_terminal;
  return SimpleChecksum(oss.str());
}

}  // namespace helpers


bool ParserBuilder::Import(const string& serialized_parser,
                           const ParserGrammar& parser_grammar,
                           Parser* parser) {
  if (not parser->IsFinalized()) {
    auto aparse_grammar_hash =
                 helpers::AdvanceParserRulesToGrammarHash(parser_grammar);
    vector<utils::any> rule_actions;
    for (auto& rule : parser_grammar.rules) {
      rule_actions.emplace_back(rule.action);
    }
    return InternalParserBuilder::Import(serialized_parser,
                            aparse_grammar_hash,
                            rule_actions,
                            parser);
  }
  return true;
}

void ParserBuilder::Build(const ParserGrammar& parser_rules,
                          Parser* parser) {
  if (not parser->IsFinalized()) {
    vector<string> rule_strings;
    for (auto& rule : parser_rules.rules) {
      rule_strings.push_back(rule.rule_string);
    }
    auto grammar = helpers::StringRulesToAParseGrammar(
                      rule_strings,
                      parser_rules.string_to_alphabet_map,
                      parser_rules.branching_alphabets,
                      parser_rules.main_non_terminal);
    vector<utils::any> rule_actions;
    for (auto& rule : parser_rules.rules) {
      rule_actions.push_back(rule.action);
    }
    InternalParserBuilder::Build(grammar, rule_actions, parser);
  }
}


std::string ParserBuilder::Export(const Parser& parser,
                                  const ParserGrammar& parser_grammar) {
  string output;
  Export(parser, parser_grammar, &output);
  return output;
}


void ParserBuilder::Export(const Parser& parser,
                           const ParserGrammar& parser_grammar,
                           std::string* serialized_parser) {
  std::size_t grammar_hash =
                    helpers::AdvanceParserRulesToGrammarHash(parser_grammar);
  InternalParserBuilder::Export(parser, grammar_hash, serialized_parser);
}

}  // namespace aparse
