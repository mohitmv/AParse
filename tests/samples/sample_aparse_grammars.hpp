// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <iostream>
#include "aparse/aparse_grammar.hpp"

#include <quick/debug.hpp>

using std::cout;
using std::endl;

namespace test {
using aparse::AParseGrammar;
using aparse::Regex;

AParseGrammar SampleGrammar1() {
  // Language: 
  //   main_non_terminal = A
  //    A ::= ( '(' A ')' )*
  // Accepted strings: "","()", "()()", "(())", "()(())", ....
  // Alphabets: (0 : '('), (1: ')')
  // NonTerminals = (2: A)
  AParseGrammar g1;
  g1.alphabet_size = 2;
  g1.rules = {
    {2, Regex(Regex::KSTAR, {Regex(Regex::CONCAT, {Regex(0),
                                                  Regex(2),
                                                  Regex(1)})})},
  };
  g1.branching_alphabets = {{0, 1}};
  g1.main_non_terminal = 2;
  return g1;
}

AParseGrammar SampleGrammar4() {
  // Language: 
  //   main_non_terminal = A
  //    A ::= 0*
  // Accepted strings: "","0", "00", "000", "0000", ....
  // Alphabets: (0 : "0")
  // NonTerminals = (1: A)
  AParseGrammar g1;
  g1.alphabet_size = 1;
  g1.rules = {
    {1, Regex(Regex::KSTAR, {Regex(0)})},
  };
  g1.branching_alphabets = {};
  g1.main_non_terminal = 1;
  return g1;
}

AParseGrammar SampleGrammar2() {
  AParseGrammar g2;
  // Language:
  //  main_non_terminal = B
  //  A ::= ('(' B ')') | NUM
  //  B ::= (A '+')* A
  // Accepted string: "4", "4+5", "3+(4+5+(3+8))+(3+4)"
  // Alphabets = (0: '(', 1: ')', 2: '+', 3: NUM)
  // NonTerminals = (4: A, 5: B)
  g2.alphabet_size = 4;
  g2.rules = {
    {4, Regex(Regex::UNION, {Regex(Regex::CONCAT, {Regex(0), Regex(5), Regex(1)}), Regex(3)})},
    {5, Regex(Regex::CONCAT, {Regex(Regex::KSTAR,
                                    {Regex(Regex::CONCAT,
                                           {Regex(4), Regex(2)})}), Regex(4)})}
  };
  g2.main_non_terminal = 5;
  g2.branching_alphabets = {{0, 1}};
  return g2;
}

AParseGrammar SampleGrammar3() {
  AParseGrammar g3;
  // Language (json):
  //  main_non_terminal = Json
  //  Json ::= '[' JsonList ']' | '{' JsonDict '}' | NUM | STRING | BOOL | NULL
  //  JsonList ::= ((Json ',')* Json)?
  //  JsonDict ::= ((JsonPair ',')* JsonPair)?
  //  JsonPair ::= STRING ':' Json
  // Accepted string: "4", "[4, 5]", "[5, 4, false, {name: mohit, age: 23}]"
  // Alphabets = {0: '[', 1: ']', 2: '{', 3: '}', 4: ',', 5: ':', 6: NUM,
  //              7: STRING, 8: BOOL, 9: NULL}
  // NonTerminals = (10: Json, 11: JsonList, 12: JsonDict, 13: JsonPair)
  g3.alphabet_size = 10;
  g3.rules = {
    {10, Regex(Regex::UNION, {Regex(Regex::CONCAT, {Regex(0), Regex(11), Regex(1)}),
                              Regex(Regex::CONCAT, {Regex(2), Regex(12), Regex(3)}),
                              Regex(6), Regex(7), Regex(8), Regex(9)})},
    {11, Regex(Regex::UNION, {Regex(Regex::EPSILON),
                              Regex(Regex::CONCAT,
                                    {Regex(Regex::KSTAR,
                                           {Regex(Regex::CONCAT, {Regex(10), Regex(4)})}),
                                     Regex(10)})})},
    {12, Regex(Regex::UNION, {Regex(Regex::EPSILON),
                              Regex(Regex::CONCAT,
                                    {Regex(Regex::KSTAR,
                                           {Regex(Regex::CONCAT, {Regex(13), Regex(4)})}),
                                     Regex(13)})})},
    {13, Regex(Regex::CONCAT, {Regex(7), Regex(5), Regex(10)})}
  };
  g3.main_non_terminal = 10;
  g3.branching_alphabets = {{0, 1}, {2, 3}};
  return g3;
}


}  // namespace test



