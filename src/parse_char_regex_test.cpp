// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/parse_char_regex.hpp"

#include <tuple>
#include "gtest/gtest.h"
#include "quick/stl_utils.hpp"

#include "aparse/regex.hpp"
#include "aparse/error.hpp"
#include "src/regex_helpers.hpp"

using std::string;
using aparse::ParseCharRegex;
using aparse::Regex;
using aparse::Error;
using std::cout;
using std::endl;

namespace helpers = aparse::helpers;
using helpers::C;
using helpers::U;
using helpers::A;
using helpers::KS;
using helpers::KP;
using helpers::E;
using helpers::R;
using uchar = unsigned char;

class ParseCharRegexTest: public ::testing::Test {
 protected:
  void SetUp() override {};
};

TEST_F(ParseCharRegexTest, Basic) {
  EXPECT_EQ(ParseCharRegex::Parse("[a-z]"), R(uchar('a'), uchar('z')+1));

  EXPECT_EQ(ParseCharRegex::Parse("[a-z]*(a|b)?"),
                       C({KS(R(uchar('a'), 1+uchar('z'))),
                          U({E(), U({Regex(uchar('a')),
                                     Regex(uchar('b'))})})}));
  {
    auto r = ParseCharRegex::Parse("group_aggregated");
    cout << r << endl;
  }
}

TEST_F(ParseCharRegexTest, ErrorReport) {
  Regex output;
  bool exception_occured = false;
  try {
    output = ParseCharRegex::Parse("[a-z[]");
  } catch (const Error& e) {
    exception_occured = true;
    EXPECT_EQ(e.status, aparse::Error::PARSING_ERROR_INVALID_TOKENS);
    EXPECT_EQ(e.error_position.first, 4);
    EXPECT_GT(e.possible_alphabets.size(), 0);
  }
  EXPECT_TRUE(exception_occured);

  Error e;
  output.Clear();
  EXPECT_FALSE(ParseCharRegex::Parse("[a-z]*(a|b)?[", &output, &e));
  EXPECT_EQ(e.status, aparse::Error::PARSING_ERROR_INCOMPLETE_TOKENS);
  EXPECT_GT(e.possible_alphabets.size(), 0);
}


TEST_F(ParseCharRegexTest, DISABLED_GoldenTest) {
  EXPECT_TRUE(false) << "Add Golden Tests";
}
