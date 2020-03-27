// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/v2/aparse_machine.hpp"

#include <quick/debug.hpp>
#include "gtest/gtest.h"

#include "tests/samples/sample_aparse_grammars.hpp"

using aparse::v2::AParseMachine;
using NFAState = AParseMachine::NFAState;
using std::cout;
using std::endl;
using std::vector;
using std::unordered_set;
using std::make_pair;
using std::pair;

TEST(NFAState, Basic) {
  NFAState s(10);
  EXPECT_TRUE(s.IsLocal());
  auto s2 = s.AddPrefix(2, 3);
  auto s3 = s2.AddPrefix(4, 2);
  EXPECT_FALSE(s2.IsLocal());
  EXPECT_EQ(10, s2.GetNumber());
  EXPECT_EQ((vector<pair<int, int>> {{4, 2}, {2, 3}}), s3.GetFullPath());
  EXPECT_EQ(1, s2.GetFullPathSize());
  EXPECT_EQ(2, s3.GetFullPathSize());
  auto s4 = s3.GetSuffix(1);
  auto s5 = s3.GetSuffix(2);
  auto s6 = s3.GetSuffix(0);
  EXPECT_EQ(s6, s3);
  EXPECT_NE(s6, s);
  EXPECT_EQ(s5, s);
  EXPECT_EQ(s4, s2);
  auto s7 = s2.AddPrefixFromOther(s3, 2);
  EXPECT_EQ(3, s7.GetFullPathSize());
  EXPECT_EQ((vector<pair<int, int>> {{4, 2}, {2, 3}, {2, 3}}),
            s7.GetFullPath());
  EXPECT_EQ(10, s7.GetNumber());
  EXPECT_EQ(make_pair(4, 2), s7.GetI(0));
  EXPECT_EQ(make_pair(2, 3), s7.GetI(1));
  EXPECT_EQ(make_pair(2, 3), s7.GetI(2));
}

