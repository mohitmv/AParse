// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/internal_parser_builder.hpp"

#include <memory>
#include <utility>

#include "src/v2/aparse_machine_builder.hpp"
#include "src/v2/core_parser.hpp"

namespace aparse {
using std::shared_ptr;
using std::unique_ptr;

namespace helpers {

vector<vector<int>> ExtractRegexAtoms(
    const vector<pair<int, Regex>>& rules) {
  std::function<void(const Regex&, vector<int>* output)> lGetTerms;
  lGetTerms = [&](const Regex& input, vector<int>* output) {
    switch (input.type) {
      case Regex::ATOMIC:
        output->push_back(input.alphabet); break;
      case Regex::UNION:
      case Regex::CONCAT:
      case Regex::KSTAR:
      case Regex::KPLUS: {
        for (auto& child : input.children) {
          lGetTerms(child, output);
        }
        break;
      }
      case Regex::EPSILON: break;
      default: assert(false); break;
    }
  };
  vector<vector<int>> output(rules.size());
  for (int i = 0; i < rules.size(); i++) {
    lGetTerms(rules[i].second, &output[i]);
  }
  return output;
}


vector<int> ExtractRuleNonTerminals(
    const std::vector<std::pair<int, Regex>>& rules) {
  vector<int> output;
  for (auto& rule : rules) {
    output.push_back(rule.first);
  }
  return output;
}


}  // namespace helpers



void InternalParserBuilder::Build(const AParseGrammar& grammar,
                                  const vector<utils::any>& rule_actions,
                                  Parser* parser) {
  if (not parser->IsFinalized()) {
    APARSE_ASSERT(grammar.Validate());
    APARSE_ASSERT(rule_actions.size() == grammar.rules.size());
    auto rule_atoms = helpers::ExtractRegexAtoms(grammar.rules);
    auto rule_non_terminals =
        helpers::ExtractRuleNonTerminals(grammar.rules);
    parser->rule_actions = rule_actions;
    parser->rule_atoms = rule_atoms;
    parser->rule_non_terminals = rule_non_terminals;
    v2::AParseMachineBuilder builder(grammar);
    auto new_machine = new v2::AParseMachine();
    builder.Build(new_machine);
    parser->machine.reset(new_machine);
    parser->Finalize();
  }
}

// format-version = 3
bool InternalParserBuilder::Import(const string& serialized_parser,
                                   std::size_t aparse_grammar_hash,
                                   const vector<utils::any>& rule_actions,
                                   Parser* parser) {
  if (serialized_parser.empty()) {
    return false;
  }
  qk::IByteStream bs;
  bs.str(serialized_parser);
  uint32_t expected_version = 3, current_version;
  bs >> current_version;
  if (expected_version != current_version) {
    return false;
  }
  std::size_t expected_aparse_grammar_hash;
  bs >> expected_aparse_grammar_hash;
  if (aparse_grammar_hash != expected_aparse_grammar_hash) {
    return false;
  }
  decltype(rule_actions.size()) rule_actions_size;
  bs >> rule_actions_size;
  if (rule_actions_size != rule_actions.size()) {
    return false;
  }
  auto new_machine = new v2::AParseMachine();
  bs >> (parser->rule_atoms) >> (parser->rule_non_terminals) >> (*new_machine);
  new_machine->initialized = true;
  parser->machine.reset(new_machine);
  parser->rule_actions = rule_actions;
  parser->Finalize();
  return true;
}


// format-version = 3
string InternalParserBuilder::Export(const Parser& parser,
                                     std::size_t aparse_grammar_hash) {
  string output;
  Export(parser, aparse_grammar_hash, &output);
  return output;
}


// format-version = 3
void InternalParserBuilder::Export(const Parser& parser,
                                   std::size_t aparse_grammar_hash,
                                   std::string* serialized_parser) {
  qk::OByteStream bs;
  uint32_t version = 3;
  auto casted_machine = static_cast<const v2::AParseMachine&>(*parser.machine);
  bs << version << aparse_grammar_hash << parser.rule_actions.size()
     << parser.rule_atoms << parser.rule_non_terminals << casted_machine;
  *serialized_parser = std::move(bs.str());
}

}  // namespace aparse
