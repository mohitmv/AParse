// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <iostream>

#include "quick/debug.hpp"
#include "quick/stl_utils.hpp"

#include "src/v2/aparse_machine_builder.hpp"
#include "src/regex_builder.hpp"

#include "tests/samples/sample_aparse_grammars.hpp"
#include <quick/file_utils.hpp>
#include <quick/time.hpp>
#include <sstream>

#include "src/parse_regex_rule.hpp"

using aparse::AParseGrammar;
using aparse::v2::AParseMachineBuilder;
using aparse::v2::AParseMachine;
using aparse::Regex;
using std::vector;
using std::set;
using aparse::v2::InternalAParseGrammar;
using std::string;
using aparse::Regex;
using NFABuilder = AParseMachineBuilder::NFABuilder;
using NFA = AParseMachineBuilder::NFA;
using NFAState = AParseMachine::NFAState;
using ParsingStream = AParseMachineBuilder::ParsingStream;
using OutgoingEdges = AParseMachineBuilder::OutgoingEdges;
using AlphabetMap = std::unordered_map<string, int>;
using R = aparse::RegexBuilderObject;

using namespace std;

using namespace aparse;

void test1() {
  std::stringstream ss(qk::ReadFile("tests/formula/parser_rules.txt"));
  vector<string> rules;
  string line;
  while (std::getline(ss, line)) {
    rules.push_back(line);
  }
  unordered_map<string, int> m;
  vector<pair<string, string>> branching_alphabets = {{"(", ")"}, {"{", "}"}, {"if", "else"}};
  auto grammar = helpers::StringRulesToAParseGrammar(rules, m, branching_alphabets, "NoAggrOrGroupAggr_any_primitive_type");
  // cout << "grammar = " << grammar << endl;
  AParseMachineBuilder builder(grammar);
  cout << "Building Machine.." << endl;
  AParseMachine machine;
  qk::MicroSecondTimer timer;
  builder.Build(&machine);
  cout << "Built, time = " << timer.GetElapsedTime() << endl;
}

int main() {
  test1();
  return 0;
}




