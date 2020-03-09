// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/regex.hpp"

#include "gtest/gtest.h"

using aparse::Regex;

TEST(RegexTest, Basic) {
  Regex r1, r2;
  r1.label = 110;
  r1.type = Regex::ATOMIC;
  r1.alphabet = 2;
  r2.label = 111;
  r2.type = Regex::KSTAR;
  r2.children.push_back(r1);
}

TEST(RegexTest, NoNewRegexType) {
  Regex r1(Regex::EPSILON);
  // Following switch-case should compile without adding more switch-case
  // or handling default.
  switch (r1.type) {
    case Regex::EPSILON: break;
    case Regex::ATOMIC: break;
    case Regex::CONCAT: break;
    case Regex::UNION: break;
    case Regex::KSTAR: break;
    case Regex::KPLUS: break;
    case Regex::ID: break;
  }
}

TEST(RegexTest, Constructors) {
  Regex r1(Regex::UNION, {Regex(12), Regex(Regex::EPSILON)});
  Regex r2(120);
  Regex r3(Regex::EPSILON);
  Regex r4(Regex::CONCAT, {r1, r2, r3});
  Regex r5(Regex::CONCAT, r4.children);
  Regex r6(Regex::CONCAT, {Regex(10), Regex(20), Regex(30)});
  EXPECT_EQ(r1.type, Regex::UNION);
  EXPECT_EQ(r1.children.size(), 2);
  EXPECT_EQ(r2.alphabet, 120);
  EXPECT_EQ(r2.type, Regex::ATOMIC);
  EXPECT_EQ(r3.type, Regex::EPSILON);
  EXPECT_EQ(r4.children.size(), 3);
  EXPECT_EQ(r4.children[1], r2);
  EXPECT_EQ(r5, r4);
  EXPECT_EQ(r1.DebugString(), "(12 | EPSILON)");
  EXPECT_EQ(r2.DebugString(), "120");
  EXPECT_EQ(r3.DebugString(), "EPSILON");
  EXPECT_EQ(r4.DebugString(), "((12 | EPSILON) 120 EPSILON)");
  EXPECT_EQ(r5.DebugString(), "((12 | EPSILON) 120 EPSILON)");
  EXPECT_EQ(r6.DebugString(), "(10 20 30)");
}

TEST(RegexTest, Operators) {
  Regex r1(Regex::UNION, {Regex(12), Regex(Regex::EPSILON)});
  auto r11 = r1;
  Regex r2(120);
  auto& r3 = r1 + r2;
  EXPECT_EQ(r3.type, Regex::CONCAT);
  EXPECT_EQ(r3.children[0], r11);
  EXPECT_EQ(r3.children[1], r2);
  auto& r4 = r3 + Regex(Regex::CONCAT, {Regex(Regex::EPSILON),
                                        Regex(20),
                                        Regex(10)});
  EXPECT_EQ(r4.children.size(), 5);
  EXPECT_EQ(r4.DebugString(), "((12 | EPSILON) 120 EPSILON 20 10)");
}
