// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/internal_lexer_builder.hpp"

#include <iostream>

#include "quick/debug.hpp"
#include "gtest/gtest.h"
#include "quick/stl_utils.hpp"

#include "src/regex_helpers.hpp"

using aparse::Lexer;
using aparse::Regex;
using aparse::helpers::RangeRegex;
using std::vector;
using std::cout;
using std::endl;
using std::pair;

using uchar = unsigned char;
enum TokenType {NUMBER, OPEN_B1, CLOSE_B1, PLUS, WHITESPACE};

struct LexerScope1: public aparse::LexerScopeBase {
  vector<TokenType> tokens;
  vector<pair<int, int>> range_list;
};

auto LexerRules1() {
  using Rule = aparse::InternalLexerGrammar::Rule;
  auto number_regex = Regex(Regex::KSTAR,
                            {RangeRegex(uchar('0'), uchar('9')+1)});
  auto whitespace_regex = Regex(Regex::KPLUS, {Regex(Regex::UNION,
                                                    {Regex(uchar(' ')),
                                                     Regex(uchar('\t')),
                                                     Regex(uchar('\n'))})});
  auto lAddAction = [&](TokenType token) {
    return [token](LexerScope1* scope) {
      scope->tokens.push_back(token);
      scope->range_list.push_back(scope->Range());
    };
  };
  aparse::InternalLexerGrammar lexer_rules;
  lexer_rules.main_section = 0;
  lexer_rules.rules = {
    {0, {
      Rule(number_regex).Action(lAddAction(NUMBER)),
      Rule(Regex(uchar('('))).Action(lAddAction(OPEN_B1)),
      Rule(Regex(uchar(')'))).Action(lAddAction(CLOSE_B1)),
      Rule(Regex(uchar('+'))).Action(lAddAction(PLUS)),
      Rule(whitespace_regex).Action(lAddAction(WHITESPACE)),
    }}
  };
  return lexer_rules;
}

static vector<TokenType> global_tokens;

auto LexerRules2() {
  using Rule = aparse::InternalLexerGrammar::Rule;
  using LexerScope = aparse::LexerScopeBase;
  auto number_regex = Regex(Regex::KSTAR,
                            {RangeRegex(uchar('0'), uchar('9')+1)});
  auto whitespace_regex = Regex(Regex::KPLUS, {Regex(Regex::UNION,
                                                    {Regex(uchar(' ')),
                                                     Regex(uchar('\t')),
                                                     Regex(uchar('\n'))})});
  auto lAddAction = [&](TokenType token) {
    return [token](LexerScope* scope) {
      global_tokens.push_back(token);
      auto r = scope->Range();
      EXPECT_GE(r.second, r.first);
    };
  };
  aparse::InternalLexerGrammar lexer_rules;
  lexer_rules.main_section = 0;
  lexer_rules.rules = {
    {0, {
      Rule(number_regex).Action(lAddAction(NUMBER)),
      Rule(Regex(uchar('('))).Action(lAddAction(OPEN_B1)),
      Rule(Regex(uchar(')'))).Action(lAddAction(CLOSE_B1)),
      Rule(Regex(uchar('+'))).Action(lAddAction(PLUS)),
      Rule(whitespace_regex).Action(lAddAction(WHITESPACE)),
    }}
  };
  return lexer_rules;
}


TEST(InternalLexerIntegrationTest, Basic) {
  Lexer lexer;
  aparse::InternalLexerBuilder::Build(LexerRules1(), &lexer);
  LexerScope1 scope;
  auto lexer_i = lexer.CreateInstance(&scope);
  {
    lexer_i.Feed("44 + (55 + 66) + 29");
    lexer_i.End();
    EXPECT_EQ(scope.tokens,
              (vector<TokenType> {NUMBER, WHITESPACE, PLUS, WHITESPACE,
                                  OPEN_B1, NUMBER, WHITESPACE, PLUS,
                                  WHITESPACE, NUMBER, CLOSE_B1, WHITESPACE,
                                  PLUS, WHITESPACE, NUMBER}));
  }
  {
    lexer_i.Reset();
    scope.tokens.clear();
    scope.range_list.clear();
    lexer_i.Feed("44+(+(55+66)+29");
    lexer_i.End();
    EXPECT_EQ(scope.tokens,
              (vector<TokenType>{NUMBER, PLUS, OPEN_B1, PLUS, OPEN_B1, NUMBER,
                                 PLUS, NUMBER, CLOSE_B1, PLUS, NUMBER}));
    auto range_list = scope.range_list;
    EXPECT_EQ(range_list[0], std::make_pair(0, 2));
    EXPECT_EQ(range_list.back(), std::make_pair(13, 15));
  }
}


TEST(DefaultScope, Basic) {
  Lexer lexer;
  aparse::InternalLexerBuilder::Build(LexerRules2(), &lexer);
  aparse::LexerScopeBase scope;
  auto lexer_i = lexer.CreateInstance(&scope);
  {
    lexer_i.Feed("44 + (55 + 66) + 29");
    lexer_i.End();
    EXPECT_EQ(global_tokens,
              (vector<TokenType> {NUMBER, WHITESPACE, PLUS, WHITESPACE,
                                  OPEN_B1, NUMBER, WHITESPACE, PLUS,
                                  WHITESPACE, NUMBER, CLOSE_B1, WHITESPACE,
                                  PLUS, WHITESPACE, NUMBER}));
  }
}

