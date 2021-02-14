// Copyright: 2020 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <iostream>

#include "quick/debug.hpp"
#include "gtest/gtest.h"

#include "aparse/aparse_machine_builder_v2.hpp"
#include "aparse/core_parser.hpp"
#include "aparse/parse_regex_rule.hpp"
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
using std::string;
using std::pair;

using namespace aparse;

using uchar = unsigned char;
using helpers::C;
using helpers::U;
using helpers::A;
using helpers::KS;
using helpers::KP;
using helpers::E;
using helpers::SU;
using helpers::SC;

namespace {

unordered_map<string, int> string_to_alphabet_map1 = {
  {"int_constant", 0},
  {"double_constant", 1},
  {"(", 2},
  {")", 3},
};


unordered_map<string, int> string_to_alphabet_map2 = {
  {"int_constant", 0},
  {"double_constant", 1},
  {"int_column", 2},
  {"double_column", 3},
  {"+", 4},
  {"*", 5},
  {"/", 6},
  {"(", 7},
  {")", 8},
};
}

AParseGrammar Grammar1() {
  vector<pair<int, int>> branching_alphabets = {{2, 3}};
  vector<string> rule_strings = {
    "main ::= int_expr+ | double_expr+",
    "int_expr ::=  int_constant | '(' int_expr ')' ",
    "double_expr ::=  double_constant | '(' double_expr ')' ",
  };
  return helpers::StringRulesToAParseGrammar(rule_strings,
                                             string_to_alphabet_map1,
                                             branching_alphabets,
                                             "main");
}


AParseGrammar Grammar2() {
  vector<pair<int, int>> branching_alphabets = {{7, 8}};
  vector<string> rule_strings = {
    "main ::= int_expr | double_expr",
    "int_expr ::= (int_atom ('+'|'*'))* int_atom",
    "double_expr ::= (double_atom ('+'|'*'|'/'))* double_atom",
    "int_atom ::= int_atom_1 | int_atom_2",
    "double_atom ::= double_atom_1 | double_atom_2",
    "int_atom_1 ::= int_constant | int_column",
    "double_atom_1 ::=   int_constant | int_column "
                      "| double_constant | double_column",
    "int_atom_2 ::= '(' int_expr ')'",
    "double_atom_2 ::= '(' double_expr ')'"
  };
  return helpers::StringRulesToAParseGrammar(rule_strings,
                                             string_to_alphabet_map2,
                                             branching_alphabets,
                                             "main");
}


class ParserGrammarValidationTest : public ::testing::Test {
 public:
  AParseMachine m;
  AParseGrammar g;
  Bug1Test() {
    g = Grammar1();
    AParseMachineBuilder b(g);
    b.Build(&m);
  }
 protected:
  void SetUp() override {}
};

TEST_F(ParserGrammarValidation, Basic) {
  cout << g.DebugString() << endl;
  cout << m.DebugString() << endl;
}

