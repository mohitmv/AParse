// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/parse_regex_rule.hpp"

#include <tuple>
#include "gtest/gtest.h"
#include "quick/stl_utils.hpp"
#include "quick/debug.hpp"
#include "src/regex_helpers.hpp"

#include "aparse/regex.hpp"
#include "aparse/error.hpp"

using std::string;
using aparse::ParseRegexRule;
using aparse::Regex;
using aparse::Error;
using std::cout;
using std::endl;
using aparse::helpers::RangeRegex;
using std::vector;
using std::pair;
using std::unordered_map;
namespace helpers = aparse::helpers;

using helpers::C;
using helpers::U;
using helpers::A;
using helpers::KS;
using helpers::KP;
using helpers::E;

using uchar = unsigned char;
using PGRule = aparse::ParsedGrammarRule;

class ParseRegexRuleTest: public ::testing::Test {
 protected:
  void SetUp() override {};
};

TEST_F(ParseRegexRuleTest, Basic) {
  vector<pair<string, PGRule>> parse_pairs = {
    {
      "Json ::= JsonList | JsonDict | STRING ",
      PGRule(PGRule::RULE,
             {PGRule(PGRule::UNION, {PGRule("JsonList"),
                                     PGRule("JsonDict"),
                                     PGRule("STRING")})})
        .SetValue("Json")
    },
    {
      "JsonList ::= OPEN_B3 (Json COMMA)* Json CLOSE_B3 ",
      PGRule(PGRule::RULE,
             {PGRule(PGRule::CONCAT,
                     {PGRule("OPEN_B3"),
                      PGRule(PGRule::KSTAR,
                             {PGRule(PGRule::CONCAT, {PGRule("Json"),
                                                      PGRule("COMMA")})}),
                      PGRule("Json"),
                      PGRule("CLOSE_B3")})
             })
        .SetValue("JsonList")
    },
  };
  for (auto& item : parse_pairs) {
    EXPECT_EQ(ParseRegexRule::Parse(item.first), item.second);
  }
}


TEST_F(ParseRegexRuleTest, DISABLED_GoldenTest) {
  EXPECT_TRUE(false) << "Add Golden Tests";
}

TEST_F(ParseRegexRuleTest, Error) {
  {
    aparse::ParsedGrammarRule o;
    Error e;
    EXPECT_FALSE(ParseRegexRule::Parse("JsonList ::= (Json COMMA)* )", &o, &e));
    EXPECT_EQ(e.status, Error::PARSING_ERROR_INVALID_TOKENS);
  }
  {
    aparse::ParsedGrammarRule o;
    Error e;
    EXPECT_TRUE(ParseRegexRule::Parse("JsonList ::= (Json COMMA)*", &o, &e));
    EXPECT_EQ(e.status, Error::SUCCESS);
  }
}


TEST(StringRulesToAParseGrammar, Basic) {
  using Alphabet = aparse::Alphabet;
  {
    bool exception_occured = false;
    vector<string> rules = {
      "Json ::= JsonList | STRING ",
      "JsonList ::= OPEN_B3 (Json COMMA)* Json CLOSE_B3 (",
    };
    unordered_map<string, Alphabet> string_to_alphabet_map = {
      {"STRING", 0},
      {"OPEN_B3", 1},
      {"CLOSE_B3", 2},
      {"COMMA", 3},
    };
    vector<pair<string, string>> branching_alphabets =
        {{"OPEN_B3", "CLOSE_B3"}};
    string main_non_terminal = "Json";
    try {
      helpers::StringRulesToAParseGrammar(rules,
                                          string_to_alphabet_map,
                                          branching_alphabets,
                                          main_non_terminal);
    } catch (const Error& e) {
      exception_occured = true;
      EXPECT_EQ(e.status, Error::PARSER_BUILDER_ERROR_INVALID_RULE);
    }
    EXPECT_TRUE(exception_occured);
  }
}

