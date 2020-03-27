// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/v2/internal_aparse_grammar.hpp"

#include <quick/debug.hpp>
#include "gtest/gtest.h"

#include "tests/samples/sample_aparse_grammars.hpp"

using aparse::v2::InternalAParseGrammar;
using std::cout;
using std::endl;
using std::vector;
using std::unordered_set;
using aparse::Regex;

TEST(InternalAParseGrammarTest, Basic) {
  {
    using IntMap = std::unordered_map<int, int>;
    auto g1 = test::SampleGrammar1();
    InternalAParseGrammar g1_internal;
    g1_internal.Init(g1);
    EXPECT_EQ(2, g1_internal.alphabet_size);
    EXPECT_EQ(2, g1_internal.main_non_terminal);
    EXPECT_EQ((IntMap {{0, 1}}), g1_internal.ba_map);
    EXPECT_EQ((IntMap {{1, 0}}), g1_internal.ba_inverse_map);
    EXPECT_EQ((IntMap {{1, 0}}),
              g1_internal.regex_label_to_original_rule_number_mapping);
    EXPECT_EQ((vector<int>{2}), g1_internal.topological_sorted_non_terminals);
    EXPECT_EQ((unordered_set<int>{2}), g1_internal.non_terminals);
    EXPECT_EQ((unordered_set<int>{3}), g1_internal.enclosed_non_terminals);
    std::unordered_map<int, Regex> rules = {{2,
                                             Regex(Regex::KSTAR,
                                                   {Regex(3)}).SetLabel(1)}};
    std::unordered_map<int, Regex> e_rules = {{3, Regex(2)}};
    std::unordered_map<int, int> e_ba = {{3, 0}};

    EXPECT_EQ(rules, g1_internal.rules);
    EXPECT_EQ(e_rules, g1_internal.enclosed_rules);
    EXPECT_EQ(e_ba, g1_internal.enclosed_ba_alphabet);
    std::unordered_map<int, std::unordered_set<int>> dg = {{2, {}}};
    EXPECT_EQ(dg, g1_internal.dependency_graph);
  }
}

TEST(InternalAParseGrammarTest, SampleGrammar3) {
  {
    using IntMap = std::unordered_map<int, int>;
    auto g3 = test::SampleGrammar3();
    InternalAParseGrammar ig;
    ig.Init(g3);
    EXPECT_EQ(10, ig.alphabet_size);
    EXPECT_EQ(10, ig.main_non_terminal);
    EXPECT_EQ((IntMap {{2, 3}, {0, 1}}), ig.ba_map);
  }
}

