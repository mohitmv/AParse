// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <iostream>

#include "quick/debug.hpp"
#include "gtest/gtest.h"

#include "samples/sample_aparse_grammars.hpp"
#include "aparse/aparse_machine_builder_v2.hpp"
#include "aparse/core_parser.hpp"
#include "aparse/error.hpp"
#include "quick/stl_utils.hpp"

using aparse::AParseGrammar;
using aparse::AParseMachineBuilder;
using aparse::CoreParser;
using aparse::CoreParseNode;
using aparse::AParseMachine;
using aparse::Regex;
using std::vector;
using std::set;

class CoreParserIntegrationTest : public ::testing::Test {
 public:
  AParseGrammar g1, g2, g3;
  AParseMachine m1, m2, m3;
  CoreParserIntegrationTest() {
    // Please Refer to `samples/sample_aparse_grammars.hpp` for the details of
    // these grammars.
    g1 = test::SampleGrammar1();
    g2 = test::SampleGrammar2();
    g3 = test::SampleGrammar3();
    AParseMachineBuilder b1(g1), b2(g2), b3(g3);
    b1.Build(&m1);
    b2.Build(&m2);
    b3.Build(&m3);
  }

 protected:
  void SetUp() override {}
};

TEST_F(CoreParserIntegrationTest, SampleGrammar1Basic) {
  CoreParser parser(&m1);
  {
    // ()()
    parser.Feed({0, 1, 0, 1});
    CoreParseNode tree;
    parser.Parse(&tree);
    CoreParseNode expected(0,
                           {0, 4},
                           {CoreParseNode(2,
                                          {0, 4},
                                          {CoreParseNode(2, {1, 1}),
                                           CoreParseNode(2, {3, 3})})});
    EXPECT_EQ(expected, tree);
  }
  {
    parser.Reset();
    // EMPTY STRING
    parser.Feed(vector<int> {});
    CoreParseNode tree;
    parser.Parse(&tree);
    CoreParseNode expected(0,
                           {0, 0},
                           {CoreParseNode(2,
                                          {0, 0})});
    EXPECT_EQ(expected, tree);
  }
  {
    parser.Reset();
    // ()()((())())
    parser.Feed({0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1});
    CoreParseNode tree;
    parser.Parse(&tree);
    CoreParseNode expected(0,
                           {0, 12},
                           {CoreParseNode(
                              2,
                              {0, 12},
                              {CoreParseNode(2, {1, 1}),
                               CoreParseNode(2, {3, 3}),
                               CoreParseNode(
                                 2,
                                 {5, 11},
                                 {CoreParseNode(2,
                                                {6, 8},
                                                {CoreParseNode(2, {7, 7})}),
                                  CoreParseNode(2, {10, 10})})})});
    EXPECT_EQ(expected, tree);
  }
}


TEST_F(CoreParserIntegrationTest, SampleGrammar2Basic) {
  CoreParser parser(&m2);
  {
    // NUM + NUM + NUM
    parser.Feed({3, 2, 3, 2, 3});
    CoreParseNode tree;
    parser.Parse(&tree);
    CoreParseNode expected(0,
                           {0, 5},
                           {CoreParseNode(5,
                                          {0, 5},
                                          {CoreParseNode(4, {0, 1}),
                                           CoreParseNode(4, {2, 3}),
                                           CoreParseNode(4, {4, 5})})});
    EXPECT_EQ(expected, tree);
  }
  {
    parser.Reset();
    // NUM + (NUM) + ((NUM+NUM)) + NUM
    parser.Feed({3, 2, 0, 3, 1, 2, 0, 0, 3, 2, 3, 1, 1, 2, 3});
    CoreParseNode tree;
    parser.Parse(&tree);
    CoreParseNode expected(0,
                           {0, 15},
                           {CoreParseNode(
                              5,
                              {0, 15},
                              {CoreParseNode(4, {0, 1}),
                               CoreParseNode(
                                 4,
                                 {2, 5},
                                 {CoreParseNode(
                                    5,
                                    {3, 4},
                                    {CoreParseNode(4, {3, 4})})}),
                               CoreParseNode(
                                 4,
                                 {6, 13},
                                 {CoreParseNode(
                                    5,
                                    {7, 12},
                                    {CoreParseNode(
                                       4,
                                       {7, 12},
                                       {CoreParseNode(
                                          5,
                                          {8, 11},
                                          {CoreParseNode(4, {8, 9}),
                                           CoreParseNode(4, {10, 11})})})})}),
                                CoreParseNode(4, {14, 15})})});
    EXPECT_EQ(expected, tree);
  }
}


TEST_F(CoreParserIntegrationTest, SampleGrammar3Basic) {
  CoreParser parser(&m3);
  {
    // [NUM, NUM]
    parser.Feed({0, 6, 4, 6, 1});
    CoreParseNode tree;
    parser.Parse(&tree);
    CoreParseNode expected(0,
                           {0, 5},
                           {CoreParseNode(
                              10,
                              {0, 5},
                              {CoreParseNode(
                                11,
                                {1, 4},
                                {CoreParseNode(10, {1, 2}),
                                 CoreParseNode(10, {3, 4})})})});
    EXPECT_EQ(expected, tree);
  }
  {
    parser.Reset();
    // [BOOL, NUM, {STRING: [NULL, {STRING: NUM}], STRING: BOOL}]
    parser.Feed({0, 8, 4, 6, 4, 2, 7, 5, 0, 9, 4, 2, 7, 5, 6, 3, 1, 4, 7, 5, 8,
                 3, 1});
    CoreParseNode tree;
    parser.Parse(&tree);
    CoreParseNode expected(0,
                           {0, 23},
                           {CoreParseNode(
                              10,
                              {0, 23},
                              {CoreParseNode(
                                11,
                                {1, 22},
                                {CoreParseNode(10, {1, 2}),
                                 CoreParseNode(10, {3, 4}),
                                 CoreParseNode(
                                   10,
                                   {5, 22},
                                   {CoreParseNode(
                                     12,
                                     {6, 21},
                                     {CoreParseNode(
                                       13,
                                       {6, 17},
                                       {CoreParseNode(
                                         10,
                                         {8, 17},
                                         {CoreParseNode(
                                           11,
                                           {9, 16},
                                           {CoreParseNode(
                                              10,
                                              {9, 10}),
                                            CoreParseNode(
                                              10,
                                              {11, 16},
                                              {CoreParseNode(
                                                12,
                                                {12, 15},
                                                {CoreParseNode(
                                                  13,
                                                  {12, 15},
                                                  {CoreParseNode(
                                                     10,
                                                     {14, 15})
                                                  })
                                                })
                                               })
                                            })
                                          })
                                        }),
                                      CoreParseNode(
                                        13,
                                        {18, 21},
                                        {CoreParseNode(
                                          10,
                                          {20, 21})
                                        })
                                    })})})})});
    EXPECT_EQ(expected, tree);
  }
}


TEST_F(CoreParserIntegrationTest, ErrorReport) {
  {
    CoreParser parser(&m1);
    // ()())()
    bool exception_occured = false;
    try {
      parser.FeedOrDie({0, 1, 0, 1, 1, 0, 1});
    } catch (const aparse::Error& e) {
      exception_occured = true;
      EXPECT_EQ(e.status, aparse::Error::PARSING_ERROR_INVALID_TOKENS);
      EXPECT_EQ(e.error_position.first, 4);
      EXPECT_EQ(e.possible_alphabets, (vector<int>{0}));
    }
    EXPECT_TRUE(exception_occured);
  }
  {
    CoreParser parser(&m3);
    bool exception_occured = false;
    try {
      parser.FeedOrDie({10, 6, 4, 6, 1});
    } catch (const aparse::Error& e) {
      exception_occured = true;
      EXPECT_EQ(e.status, aparse::Error::PARSING_ERROR_INVALID_TOKENS);
      EXPECT_EQ(e.error_position.first, 0);
      EXPECT_EQ(qk::ToSet(e.possible_alphabets), (set<int> {6, 7, 8, 9, 0, 2}));
    }
    EXPECT_TRUE(exception_occured);
  }
  {
    CoreParser parser(&m1);
    // ()()(((
    bool exception_occured = false;
    CoreParseNode tree;
    try {
      parser.FeedOrDie({0, 1, 0, 1, 0, 0, 0});
      parser.ParseOrDie(&tree);
    } catch (const aparse::Error& e) {
      exception_occured = true;
      EXPECT_EQ(e.status, aparse::Error::PARSING_ERROR_INCOMPLETE_TOKENS);
      EXPECT_EQ(qk::ToSet(e.possible_alphabets), (set<int>{0, 1}));
    }
    EXPECT_TRUE(exception_occured);
  }
}


TEST_F(CoreParserIntegrationTest, Serializer) {
  {
    qk::OByteStream obs;
    obs << m3;
    std::string serialized_machine = obs.str();
    qk::IByteStream ibs;
    ibs.str(std::move(serialized_machine));
    AParseMachine m3_imported;
    ibs >> m3_imported;
    EXPECT_EQ(m3, m3_imported);
    CoreParser parser(&m3_imported);
    // [NUM, NUM]
    parser.Feed({0, 6, 4, 6, 1});
    CoreParseNode tree;
    parser.Parse(&tree);
    CoreParseNode expected(0,
                           {0, 5},
                           {CoreParseNode(
                              10,
                              {0, 5},
                              {CoreParseNode(
                                11,
                                {1, 4},
                                {CoreParseNode(10, {1, 2}),
                                 CoreParseNode(10, {3, 4})})})});
    EXPECT_EQ(expected, tree);
  }
}

TEST_F(CoreParserIntegrationTest, ImportExport) {
  std::string s1 = m1.Export(), s2 = m2.Export(), s3 = m3.Export();
  AParseMachine m11, m22, m33;
  m11.Import(s1);
  m22.Import(s2);
  m33.Import(s3);
  EXPECT_EQ(s1.size(), 1386);
  EXPECT_EQ(s2.size(), 1984);
  EXPECT_EQ(s3.size(), 5900);
  EXPECT_EQ(m1, m11);
  EXPECT_EQ(m2, m22);
  EXPECT_EQ(m3, m33);
}



