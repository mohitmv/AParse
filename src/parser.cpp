// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/parser.hpp"

#include <memory>

#include "quick/utility.hpp"

#include "src/abstract_core_parser.hpp"
#include "src/v2/core_parser.hpp"

namespace aparse {

const vector<Alphabet>& ParserInstance::GetStream() const {
  APARSE_DEBUG_ASSERT(core_parser != nullptr);
  return core_parser->GetStream();
}

void ParserInstance::Reset() {
  core_parser->Reset();
  parse_tree = CoreParseNode();
}

bool ParserInstance::Feed(Alphabet a) {
  return core_parser->Feed(a);
}

bool ParserInstance::Feed(Alphabet a, Error* error) {
  return core_parser->Feed(a, error);
}

void ParserInstance::FeedOrDie(Alphabet a) {
  core_parser->FeedOrDie(a);
}

bool ParserInstance::Feed(const vector<Alphabet>& s) {
  return core_parser->Feed(s);
}

bool ParserInstance::Feed(const vector<Alphabet>& s, Error* error) {
  return core_parser->Feed(s, error);
}

void ParserInstance::FeedOrDie(const vector<Alphabet>& s) {
  core_parser->FeedOrDie(s);
}

bool ParserInstance::End() {
  return core_parser->Parse(&parse_tree);
}

bool ParserInstance::End(Error* error) {
  return core_parser->Parse(&parse_tree, error);
}

void ParserInstance::EndOrDie() {
  core_parser->ParseOrDie(&parse_tree);
}

bool ParserInstance::IsFinal() const {
  return core_parser->IsFinal();
}

unordered_set<Alphabet> ParserInstance::PossibleAlphabets(int k) const {
  return core_parser->PossibleAlphabets();
}

unordered_set<Alphabet> ParserInstance::PossibleAlphabets() const {
  return core_parser->PossibleAlphabets();
}



ParserInstance::ParserInstance(const Parser& parser) {
  this->Init(parser);
}

void ParserInstance::Init(const Parser& parser) {
  APARSE_ASSERT(parser.IsFinalized());
  core_parser.reset(new aparse::v2::CoreParser(parser.machine.get()));
  syntax_tree_maker = parser.syntax_tree_maker.get();
}

bool Parser::Finalize() {
  (void)machine_type;
  APARSE_ASSERT(machine != nullptr);
  APARSE_ASSERT(rule_actions.size() > 0);
  APARSE_ASSERT(rule_actions.size() == rule_atoms.size());
  APARSE_ASSERT(rule_actions.size() == rule_non_terminals.size());
  syntax_tree_maker.reset(new SyntaxTreeMaker(rule_actions,
                                              rule_atoms,
                                              rule_non_terminals));
  is_finalized = true;
  return true;
}

}  // namespace aparse
