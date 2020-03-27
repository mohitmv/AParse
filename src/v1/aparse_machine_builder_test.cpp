// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/aparse_machine_builder_v2.hpp"

#include <iostream>
#include "gtest/gtest.h"
#include "quick/debug.hpp"

#include "samples/sample_aparse_grammars.hpp"

using aparse::AParseGrammar;
using aparse::AParseMachineBuilder;
using aparse::AParseMachine;
using aparse::Regex;
using std::unordered_map;
using std::pair;

class AParseMachineBuilderTest : public ::testing::Test {
 public:
  AParseGrammar g1, g2, g3;
  AParseMachineBuilderTest() {
    g1 = test::SampleGrammar1();
    g2 = test::SampleGrammar2();
  }

 protected:
  void SetUp() override {}
};


TEST_F(AParseMachineBuilderTest, Step1BuildInternalGrammar) {
  using DummySubRegexMap = unordered_map<int, pair<pair<int, int>, Regex>>;
  using InternalGrammar = AParseMachineBuilder::InternalGrammar;
  auto lToDummySubRegexMap = [](const InternalGrammar& igrammar) {
    DummySubRegexMap output;
    for (auto& item : igrammar.sub_regex_map) {
      output[item.first] = {
                            { item.second.branch_start_alphabet,
                              item.second.branch_close_alphabet
                            }, item.second.regex
                          };
    }
    return output;
  };
  {
    AParseMachineBuilder builder(g1);
    builder.Step1BuildInternalGrammar();
    Regex e_main_regex = std::move(Regex(Regex::KSTAR, {Regex(2)}).SetLabel(2));
    DummySubRegexMap e_sub_regex_map = {{2, {{0, 1}, e_main_regex}}};
    EXPECT_EQ(e_main_regex, builder.igrammar.main_regex);
    EXPECT_EQ(e_sub_regex_map, lToDummySubRegexMap(builder.igrammar));
  }
  {
    AParseMachineBuilder builder(g2);
    builder.Step1BuildInternalGrammar();
    Regex e_main_regex = std::move(Regex(Regex::KSTAR, {Regex(2)}).SetLabel(2));
    DummySubRegexMap e_sub_regex_map = {{2, {{0, 1}, e_main_regex}}};
    EXPECT_EQ(builder.igrammar.main_regex.DebugString(),
              "((((((5 | 3))[4] 2))* ((5 | 3))[4]))[5]");
    EXPECT_EQ(qk::ToString(lToDummySubRegexMap(builder.igrammar)),
              "{5: ((0, 1), ((((((5 | 3))[4] 2))* ((5 | 3))[4]))[5])}");
  }
}



TEST_F(AParseMachineBuilderTest, Basic) {
  AParseMachineBuilder builder(g1);
  auto machine = builder.Build();
  EXPECT_GT(machine.DebugString().size(), 0);
}



