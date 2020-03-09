// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <iostream>

#include "gtest/gtest.h"

#include "quick/debug.hpp"
#include "quick/stl_utils.hpp"

#include "aparse/lexer.hpp"
#include "aparse/lexer_builder.hpp"
#include "aparse/parser.hpp"
#include "aparse/parser_builder.hpp"

using aparse::Lexer;
using aparse::Parser;
using aparse::ParserInstance;
using aparse::ParserGrammar;
using aparse::ParserBuilder;
using std::vector;
using std::string;
using std::pair;
using std::cout;
using std::endl;


struct Expression {
  enum Type {NUMBER, PLUS, STAR};
  Type type;
  int value;
  vector<Expression> children;
  int Eval() {
    switch (type) {
      case NUMBER:
        return value;
      case PLUS: {
        int answer = 0;
        for (auto& item : children) {
          answer += item.Eval();
        }
        return answer;
      }
      case STAR: {
        int answer = 1;
        for (auto& item : children) {
          answer *= item.Eval();
        }
        return answer;
      }
      default: assert(false);
    }
    return 0;
  }
};


using uchar = unsigned char;

struct LexerScope: public aparse::LexerScopeBase {
  enum TokenType {NUMBER, OPEN_B1, CLOSE_B1, PLUS, STAR};
  using Token = pair<TokenType, string>;
  vector<Token> tokens;
  const string* content;
};

struct ParserScope: aparse::ParserScopeBase<Expression> {
  const vector<LexerScope::Token>* tokens;
};

void BuildLexer(aparse::Lexer* lexer) {
  using Rule = aparse::LexerGrammar::Rule;
  using TokenType = LexerScope::TokenType;
  aparse::LexerGrammar lexer_rules;

  auto lAddAction = [](TokenType token) {
    return [token](LexerScope* scope) {
      auto r = scope->Range();
      auto&& str = scope->content->substr(r.first, r.second - r.first);
      scope->tokens.push_back({token, str});
    };
  };
  lexer_rules.main_section = 0;
  lexer_rules.rules = {
    {0, {
      Rule("[0-9]+").Action(lAddAction(LexerScope::NUMBER)),
      Rule("\\(").Action(lAddAction(LexerScope::OPEN_B1)),
      Rule("\\)").Action(lAddAction(LexerScope::CLOSE_B1)),
      Rule("\\+").Action(lAddAction(LexerScope::PLUS)),
      Rule("\\*").Action(lAddAction(LexerScope::STAR)),
      Rule(" |\t|\n"),
    }}
  };
  aparse::LexerBuilder::Build(lexer_rules, lexer);
}

ParserGrammar MyGrammar() {
  using Rule = ParserGrammar::Rule;
  ParserGrammar grammar;
  grammar.branching_alphabets = {{"OPEN_B1", "CLOSE_B1"}};
  grammar.string_to_alphabet_map = {
                                  {"PLUS", LexerScope::PLUS},
                                  {"STAR", LexerScope::STAR},
                                  {"NUMBER", LexerScope::NUMBER},
                                  {"OPEN_B1", LexerScope::OPEN_B1},
                                  {"CLOSE_B1", LexerScope::CLOSE_B1},
                                };
  grammar.main_non_terminal = "<main>";
  grammar.rules = {
    Rule("<main> ::= (<multiplied> PLUS)* <multiplied>")
      .Action([](ParserScope* scope, Expression* output) {
        if (scope->ValueList().size() == 1) {
          *output = std::move(scope->ValueList()[0]);
        } else {
          output->type = Expression::PLUS;
          output->children = std::move(scope->ValueList());
        }
      }),
    Rule("<atom> ::= NUMBER | OPEN_B1 <main> CLOSE_B1 ")
      .Action([](ParserScope* scope, Expression* output) {
        if (scope->Exists(0)) {
          output->type = Expression::NUMBER;
          output->value = std::stoi(scope->tokens->at(
                                              scope->AlphabetIndex(0)).second);
        } else {
          *output = std::move(scope->Value());
        }
      }),
    Rule("<multiplied> ::= (<atom> STAR)* <atom>")
      .Action([](ParserScope* scope, Expression* output) {
        if (scope->ValueList().size() == 1) {
          *output = std::move(scope->ValueList()[0]);
        } else {
          output->type = Expression::STAR;
          output->children = std::move(scope->ValueList());
        }
      })
  };
  return grammar;
}

void BuildParser(Parser* parser) {
  ParserBuilder::Build(MyGrammar(), parser);
}


class ParserBuilderIntegrationTest : public ::testing::Test {
 public:
  ParserBuilderIntegrationTest() {
    BuildLexer(&lexer_main);
    BuildParser(&parser_main);
  }

 protected:
  Lexer lexer_main;
  Parser parser_main;
  void SetUp() override {}
  Expression Parse(const string& content) {
    auto parser = parser_main.CreateInstance();
    return Parse(content, parser);
  }

  Expression Parse(const string& content, ParserInstance& p) {  // NOLINT
    LexerScope lexer_scope;
    auto lexer = lexer_main.CreateInstance(&lexer_scope);
    lexer_scope.content = &content;
    for (auto c : content) {
      EXPECT_TRUE(lexer.Feed(uchar(c)));
    }
    EXPECT_TRUE(lexer.End());
    auto tokens = lexer_scope.tokens;
    p.Reset();
    for (auto token : tokens) {
      EXPECT_TRUE(p.Feed(token.first));
    }
    EXPECT_TRUE(p.End());
    ParserScope scope;
    scope.tokens = &tokens;
    Expression output;
    p.CreateSyntaxTree(&scope, &output);
    return output;
  }
};


TEST_F(ParserBuilderIntegrationTest, Basic) {
  EXPECT_EQ(Parse("3+4+6").Eval(), 13);
  EXPECT_EQ(Parse("3+5*6").Eval(), 33);
  EXPECT_EQ(Parse("3*5+6").Eval(), 21);
  EXPECT_EQ(Parse("3*(5+6)").Eval(), 33);
  EXPECT_EQ(Parse("3*(5+(5+2+4+(22+5)+(33))+44)").Eval(), 360);
  EXPECT_EQ(Parse(" 433 + 88 *( 445 + 99 + 44 * 8 +(22 + 5)+( 3+ 33))+544")
                  .Eval(), 85369);
}

TEST_F(ParserBuilderIntegrationTest, DISABLED_ImportExport) {
  auto grammar = MyGrammar();
  string parser_export = ParserBuilder::Export(parser_main, grammar);
  Parser p1;
  EXPECT_TRUE(ParserBuilder::Import(parser_export, grammar, &p1));
  auto p1_i = p1.CreateInstance();
  EXPECT_EQ(Parse("3+4+6", p1_i).Eval(), 13);
  EXPECT_EQ(Parse(" 433 + 88 *( 445 + 99 + 44 * 8 +(22 + 5)+( 3+ 33))+544",
                  p1_i).Eval(), 85369);
  EXPECT_EQ(parser_export.size(), 4922);
  EXPECT_EQ(parser_export.size(), ParserBuilder::Export(p1, grammar).size());
}


TEST_F(ParserBuilderIntegrationTest, ParserInstance) {
  auto& pi = parser_main;
  ParserInstance p1(pi), p2(pi), p3(pi);
  for (auto& item : {&p1, &p2, &p3}) {
    auto& p = *item;
    EXPECT_EQ(Parse("3+4+6", p).Eval(), 13);
    EXPECT_EQ(Parse(" 433 + 88 *( 445 + 99 + 44 * 8 +(22 + 5)+( 3+ 33))+544",
                    p).Eval(), 85369);
    EXPECT_EQ(Parse("3*(5+6)", p).Eval(), 33);
    EXPECT_EQ(Parse("3*(5+(5+2+4+(22+5)+(33))+44)", p).Eval(), 360);
  }
}
