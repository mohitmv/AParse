#include <aparse/aparse.hpp>

#include <vector>
#include <utility>

#include "gtest/gtest.h"

using std::vector;
using std::unordered_map;
using std::pair;
using std::string;

struct LexerScope : aparse::AdvanceLexerScopeBase {
  enum TokenType {S1, S2, S3, MOHIT, SAINI, DOUBLE_CONSTANT, INT_CONSTANT,
                  BOOL_CONSTANT, LITERAL, STRING_CONSTANT, ANNOTATION_NAME};
  using Token = pair<TokenType, pair<int, int>>;
  vector<Token> tokens;
};

using Lexer = aparse::AdvanceLexer<LexerScope>;
using TokenType = LexerScope::TokenType;
using Token = LexerScope::Token;

void BuildLexer(Lexer *lexer) {
  using Rule = Lexer::Rule;
  Lexer::LexerRules lexer_rules;
  auto lAdd = [](TokenType token_type) {
    return ([token_type](LexerScope* scope) {
      scope->tokens.emplace_back(Token {token_type, scope->Range()});
    });
  };
  std::unordered_map<string, TokenType> keywords = {
      {"mohit", TokenType::MOHIT},
      {"saini", TokenType::SAINI},
      {"group_aggregated", TokenType::S1}};
  std::vector<Rule> rules;
  for (auto& item: keywords) {
    rules.emplace_back(
      Rule(item.first).Action(lAdd(item.second)));
  }
  rules.emplace_back(
    Rule("[0-9]+\\.[0-9]+").Action(lAdd(TokenType::DOUBLE_CONSTANT)));
  rules.emplace_back(
    Rule("[0-9]+").Action(lAdd(TokenType::INT_CONSTANT)));
  rules.emplace_back(
    Rule("true|false").Action(lAdd(TokenType::BOOL_CONSTANT)));
  rules.emplace_back(
    Rule("[a-zA-Z_][a-zA-Z0-9_]*").Action(lAdd(TokenType::LITERAL)));
  rules.emplace_back(Rule("( |\t|\n)+"));
  rules.emplace_back(Rule("//[^\n]*"));
  rules.emplace_back(
    Rule("@[a-zA-Z_][a-zA-Z0-9_]*").Action(lAdd(TokenType::ANNOTATION_NAME)));
  string regex_string_1 = "'(([^\\\\'\t\n])|(\\\\[^]))*'";
  string regex_string_2 = "\"(([^\\\\\"\t\n])|(\\\\[^]))*\"";
  rules.emplace_back(
    Rule(regex_string_1 + "|" + regex_string_2)
      .Action(lAdd(TokenType::STRING_CONSTANT)));
  lexer_rules.rules[0] = std::move(rules);
  lexer_rules.main_section = 0;
  lexer->Build(lexer_rules);
}

TEST(BuildLexerTest, Basic) {
  Lexer lexer;
  BuildLexer(&lexer);
}

