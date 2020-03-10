// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/v2/aparse_machine_builder.hpp"

#include <quick/debug.hpp>
#include "gtest/gtest.h"

#include "src/regex_builder.hpp"

#include "tests/samples/sample_aparse_grammars.hpp"

using aparse::v2::InternalAParseGrammar;
using aparse::v2::AParseMachineBuilder;
using aparse::v2::AParseMachine;
using aparse::AParseGrammar;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::unordered_set;
using aparse::Regex;
using NFABuilder = AParseMachineBuilder::NFABuilder;
using NFA = AParseMachineBuilder::NFA;
using NFAState = AParseMachine::NFAState;
using ParsingStream = AParseMachineBuilder::ParsingStream;
using OutgoingEdges = AParseMachineBuilder::OutgoingEdges;

class NFABuilderTest : public ::testing::Test {
 public:
  using B = AParseMachine::BranchSymbolType;
  template<typename T> using NFAStateMap = qk::unordered_map<NFAState, T>;
  using NFAStateSet = qk::unordered_set<NFAState>;
  using FinalStatesMap = NFAStateMap<ParsingStream>;
  NFABuilderTest() : builder(nfa_map, rules, &nfa_instance_map, &counter) {}
  std::unordered_map<int, NFA> nfa_map;
  std::unordered_map<int, Regex> rules;
  std::unordered_map<int, int> nfa_instance_map;
  int counter = 0;
  NFABuilder builder;
};

// Testing NFABuilder for atomic regex.
TEST_F(NFABuilderTest, AtomicRegex) {
  auto r = Regex(3).SetLabel(10);
  NFA nfa;
  builder.Build(r, &nfa);
  EXPECT_EQ(nfa.start_state, NFAState(0));
  ParsingStream ps1 = {{B::BRANCH_END_MARKER, 10}};
  NFAStateMap<ParsingStream> final_states = {{NFAState(1), ps1}};
  EXPECT_EQ(nfa.final_states, final_states);
  ParsingStream ps2 = {{B::BRANCH_START_MARKER, 10}};
  NFAStateMap<OutgoingEdges> edges = {
    {NFAState(0), {{3, {{NFAState(1), ps2}}}}}};
  EXPECT_EQ(nfa.edges, edges);
}

// Testing NFABuilder for Epsilon Regex.
TEST_F(NFABuilderTest, EpsilonRegex) {
  auto r = Regex(Regex::EPSILON).SetLabel(10);
  NFA nfa;
  builder.Build(r, &nfa);
  EXPECT_EQ(nfa.start_state, NFAState(0));
  ParsingStream ps = {{B::BRANCH_START_MARKER, 10},
                      {B::BRANCH_END_MARKER, 10}};
  NFAStateMap<ParsingStream> final_states = {{NFAState(0), ps}};
  EXPECT_EQ(nfa.final_states, final_states);
  EXPECT_EQ(0, nfa.edges.size());
}

// Testing NFABuilder for KleeStar Regex.
TEST_F(NFABuilderTest, KStarRegex) {
  auto r = Regex(Regex::KSTAR, {Regex(3).SetLabel(15)}).SetLabel(17);  // (3*)
  NFA nfa;
  builder.Build(r, &nfa);
  ParsingStream ps1 = {{B::BRANCH_START_MARKER, 17},
                       {B::BRANCH_END_MARKER, 17}};
  ParsingStream ps2 = {{B::BRANCH_END_MARKER, 15},
                       {B::BRANCH_END_MARKER, 17}};
  ParsingStream ps3 = {{B::BRANCH_START_MARKER, 17},
                       {B::BRANCH_START_MARKER, 15}};
  ParsingStream ps4 = {{B::BRANCH_END_MARKER, 15},
                       {B::BRANCH_START_MARKER, 15}};
  EXPECT_EQ(nfa.start_state, NFAState(0));
  NFAStateMap<ParsingStream> final_states = {{NFAState(0), ps1},
                                             {NFAState(2), ps2}};
  EXPECT_EQ(final_states, nfa.final_states);
  NFAStateMap<OutgoingEdges> edges = {
      {NFAState(0), {{3, {{NFAState(2), ps3}}}}},
      {NFAState(2), {{3, {{NFAState(2), ps4}}}}}};
  EXPECT_EQ(edges, nfa.edges);
}

// Testing NFABuilder for KleeStar Regex.
TEST_F(NFABuilderTest, DISABLED_UnionRegex) {
  // (1|2|3)
  auto r = Regex(Regex::UNION, {Regex(1), Regex(2), Regex(3)}).SetLabel(17);
  NFA nfa;
  builder.Build(r, &nfa);
}

TEST(BuildSampleGrammar, Grammar1) {
  auto g = test::SampleGrammar1();
  AParseMachineBuilder builder(g);
  AParseMachine machine;
  builder.Build(&machine);
}

TEST(BuildSampleGrammar, Grammar2) {
  auto g = test::SampleGrammar2();
  AParseMachineBuilder builder(g);
  AParseMachine machine;
  builder.Build(&machine);
}

