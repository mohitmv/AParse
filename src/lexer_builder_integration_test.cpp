// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/lexer_builder.hpp"

#include <iostream>

#include <memory>
#include "quick/debug.hpp"
#include "gtest/gtest.h"
#include "quick/stl_utils.hpp"
#include "quick/utility.hpp"

using aparse::Lexer;
using aparse::LexerInstance;
using aparse::LexerGrammar;
using std::vector;
using std::cout;
using std::endl;
using std::unique_ptr;

enum TokenType {NUMBER, OPEN_B1, CLOSE_B1, PLUS, WHITESPACE};

struct LexerScope1: public aparse::LexerScopeBase {
  vector<TokenType> tokens;
};

auto LexerRules1() {
  using Rule = aparse::LexerGrammar::Rule;
  aparse::LexerGrammar lexer_rules;
  auto lAddAction = [](TokenType token) {
    return [token](LexerScope1* scope) {
      scope->tokens.push_back(token);
    };
  };
  lexer_rules.main_section = 0;
  lexer_rules.rules = {
    {0, {
      Rule("[0-9]+").Action(lAddAction(NUMBER)),
      Rule("\\(").Action(lAddAction(OPEN_B1)),
      Rule("\\)").Action(lAddAction(CLOSE_B1)),
      Rule("\\+").Action(lAddAction(PLUS)),
      Rule(" |\t|\n").Action(lAddAction(WHITESPACE)),
    }}
  };
  return lexer_rules;
}


TEST(AdvanceLexerIntegrationTest, Basic) {
  aparse::Lexer lexer_main;
  aparse::LexerBuilder::Build(LexerRules1(), &lexer_main);
  LexerScope1 scope;
  auto lexer = lexer_main.CreateInstance(&scope);
  {
    lexer.FeedOrDie("44 + (55 + 66) + 29");
    lexer.EndOrDie();
    EXPECT_EQ(scope.tokens,
              (vector<TokenType> {NUMBER, WHITESPACE, PLUS, WHITESPACE,
                                  OPEN_B1, NUMBER, WHITESPACE, PLUS,
                                  WHITESPACE, NUMBER, CLOSE_B1, WHITESPACE,
                                  PLUS, WHITESPACE, NUMBER}));
  }
  {
    lexer.Reset();
    scope.tokens.clear();
    lexer.FeedOrDie("44+(+(55+66)+29");
    lexer.EndOrDie();
    EXPECT_EQ(scope.tokens,
              (vector<TokenType>{NUMBER, PLUS, OPEN_B1, PLUS, OPEN_B1, NUMBER,
                                 PLUS, NUMBER, CLOSE_B1, PLUS, NUMBER}));
  }
}
