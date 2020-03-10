// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <iostream>

#include "quick/debug.hpp"
#include "gtest/gtest.h"
#include "quick/stl_utils.hpp"

#include "aparse/parser.hpp"
#include "src/internal_parser_builder.hpp"
#include "src/v2/aparse_machine.hpp"

#include "tests/samples/sample_internal_parser_rules.hpp"

using aparse::Parser;
using aparse::ParserInstance;
using aparse::InternalParserBuilder;
using std::vector;
using std::set;
using std::string;
using std::cout;
using std::endl;
using std::unordered_set;
using aparse::Alphabet;
using test::Grammar1Node;
using test::Grammar3Node;

TEST(InternalParserBuilderIntegrationTest, Builder) {
  aparse::Parser parser;
  auto g1 = test::Grammar1();
  InternalParserBuilder::Build(g1.aparse_grammar, g1.rule_actions, &parser);
  EXPECT_TRUE(parser.IsFinalized());
}

TEST(InternalParserBuilderIntegrationTest, SampleGrammar1Basic) {
  aparse::Parser parser_main;
  auto g1 = test::Grammar1();
  InternalParserBuilder::Build(g1.aparse_grammar,
                               g1.rule_actions,
                               &parser_main);
  auto parser = parser_main.CreateInstance();
  {
    // ()()
    EXPECT_TRUE(parser.Feed(g1.MakeStream({"(", ")", "(", ")"})));
    EXPECT_TRUE(parser.End());
    Grammar1Node syntax_tree;
    parser.CreateSyntaxTree(&syntax_tree);
    EXPECT_EQ(syntax_tree.DebugString(), "()()");
  }
  {
    parser.Reset();
    // <EMPTY STRING>
    EXPECT_TRUE(parser.Feed(vector<int> {}));
    EXPECT_TRUE(parser.End());
    Grammar1Node syntax_tree;
    parser.CreateSyntaxTree(&syntax_tree);
    EXPECT_EQ(syntax_tree.DebugString(), "");
  }
  {
    parser.Reset();
    // ()()((())())
    EXPECT_TRUE(parser.Feed(g1.MakeStream({
        "(", ")", "(", ")", "(", "(", "(", ")", ")", "(", ")", ")"})));
    EXPECT_TRUE(parser.End());
    Grammar1Node syntax_tree;
    parser.CreateSyntaxTree(&syntax_tree);
    EXPECT_EQ(syntax_tree.DebugString(), "()()((())())");
  }
}


TEST(InternalParserIntegrationTest, SampleGrammar3Basic) {
  aparse::Parser parser_main;
  auto g3 = test::Grammar3();
  InternalParserBuilder::Build(g3.aparse_grammar,
                               g3.rule_actions,
                               &parser_main);
  auto p3_i = parser_main.CreateInstance();
  {
    p3_i.Reset();
    // [NUM, NUM]
    EXPECT_TRUE(p3_i.Feed(g3.MakeStream({"[", "NUM", ",", "NUM", "]"})));
    EXPECT_TRUE(p3_i.End());
    Grammar3Node syntax_tree;
    p3_i.CreateSyntaxTree(&syntax_tree);
    EXPECT_EQ(syntax_tree.DebugString(), "[NUM, NUM]");
  }
  {
    p3_i.Reset();
    EXPECT_TRUE(p3_i.Feed(g3.MakeStream({
      "[", "BOOL", ",", "NUM", ",", "{", "STRING", ":", "[", "NULL", ",", "{",
      "STRING", ":", "NUM", "}", "]", ",", "STRING", ":", "BOOL", "}", "]"})));
    EXPECT_TRUE(p3_i.End());
    Grammar3Node syntax_tree;
    p3_i.CreateSyntaxTree(&syntax_tree);
    EXPECT_EQ(syntax_tree.DebugString(),
              "[BOOL, NUM, {STRING: BOOL, STRING: [NULL, {STRING: NUM}]}]");
  }
}

TEST(InternalParserIntegrationTest, SampleGrammar3ErrorReport) {
  aparse::Parser parser_main;
  auto g3 = test::Grammar3();
  InternalParserBuilder::Build(g3.aparse_grammar,
                               g3.rule_actions,
                               &parser_main);
  auto s =  [&](const string& a) { return g3.string_map.at(a); };

  auto p3_i = parser_main.CreateInstance();
  {
    p3_i.Reset();
    // [, NUM, {STRING: [NULL, {STRING: NUM}], STRING: BOOL}]
    aparse::Error e;
    bool success = p3_i.Feed(g3.MakeStream({
      "[", ",", "NUM", ",", "{", "STRING", ":", "[", "NULL", ",", "{",
      "STRING", ":", "NUM", "}", "]", ","}), &e);
    EXPECT_EQ(e.status, aparse::Error::PARSING_ERROR_INVALID_TOKENS);
    EXPECT_EQ(e.error_position.first, 1);
    unordered_set<Alphabet> possible_alphabets = {
      s("{"), s("["), s("]"), s("NUM"), s("STRING"), s("NULL"), s("BOOL")};
    EXPECT_EQ(possible_alphabets, e.possible_alphabets);
    EXPECT_FALSE(success);
  }
  {
    p3_i.Reset();
    // [BOOL, NUM, {STRING: [NULL, {STRING: NUM}], STRING: BOOL
    aparse::Error e;
    EXPECT_TRUE(p3_i.Feed(g3.MakeStream({
      "[", "BOOL", ",", "NUM", ",", "{", "STRING", ":", "[", "NULL", ",", "{",
      "STRING", ":", "NUM"}), &e));
    ASSERT_TRUE(e.Ok());
    ASSERT_FALSE(p3_i.End(&e));
    EXPECT_EQ(e.status, aparse::Error::PARSING_ERROR_INCOMPLETE_TOKENS);
    unordered_set<Alphabet> possible_alphabets = {s("}"), s(",")};
    EXPECT_EQ(possible_alphabets, e.possible_alphabets);
  }
}


TEST(InternalParserIntegrationTest, ParserInstance) {
  aparse::Parser parser_main;
  auto g3 = test::Grammar3();
  InternalParserBuilder::Build(g3.aparse_grammar,
                               g3.rule_actions,
                               &parser_main);
  ParserInstance p31(parser_main), p32(parser_main), p33(parser_main);
  for (auto& item : {&p31, &p32, &p33}) {
    auto& p = *item;
    for (int i = 0; i < 2; i++) {
      p.Reset();
      EXPECT_TRUE(p.Feed(g3.MakeStream({
        "[", "BOOL", ",", "NUM", ",", "{", "STRING", ":", "[", "NULL", ",", "{",
        "STRING", ":", "NUM", "}", "]", ",", "STRING", ":", "BOOL", "}",
        "]"})));
      EXPECT_TRUE(p.End());
      Grammar3Node syntax_tree;
      p.CreateSyntaxTree(&syntax_tree);
      EXPECT_EQ(syntax_tree.DebugString(),
                "[BOOL, NUM, {STRING: BOOL, STRING: [NULL, {STRING: NUM}]}]");
    }
  }
}

TEST(InternalParserIntegrationTest, ImportExport) {
  aparse::Parser parser_main;
  auto g3 = test::Grammar3();
  InternalParserBuilder::Build(g3.aparse_grammar,
                               g3.rule_actions,
                               &parser_main);
  std::size_t grammar_hash = qk::HashFunction(g3.aparse_grammar);
  string p3_export = InternalParserBuilder::Export(parser_main, grammar_hash);
  EXPECT_EQ(p3_export.size(), 3452);
  Parser p33, p333;
  EXPECT_TRUE(InternalParserBuilder::Import(p3_export,
                                            grammar_hash,
                                            g3.rule_actions,
                                            &p33));
  string p33_export = InternalParserBuilder::Export(p33, grammar_hash);
  EXPECT_TRUE(InternalParserBuilder::Import(p33_export,
                                            grammar_hash,
                                            g3.rule_actions,
                                            &p333));
  EXPECT_EQ(p33_export.size(), p3_export.size());
  EXPECT_EQ(InternalParserBuilder::Export(p333, grammar_hash).size(),
            p33_export.size());
  auto p33_i = p33.CreateInstance();
  auto p333_i = p333.CreateInstance();
  for (auto& item : {&p333_i, &p33_i}) {
    auto& p = *item;
    for (int i = 0; i < 2; i++) {
      p.Reset();
      EXPECT_TRUE(p.Feed(g3.MakeStream({
        "[", "BOOL", ",", "NUM", ",", "{", "STRING", ":", "[", "NULL", ",", "{",
        "STRING", ":", "NUM", "}", "]", ",", "STRING", ":", "BOOL", "}",
        "]"})));
      EXPECT_TRUE(p.End());
      Grammar3Node syntax_tree;
      p.CreateSyntaxTree(&syntax_tree);
      EXPECT_EQ(syntax_tree.DebugString(),
                "[BOOL, NUM, {STRING: BOOL, STRING: [NULL, {STRING: NUM}]}]");
    }
  }
}
