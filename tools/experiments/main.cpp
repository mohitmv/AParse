#include "aparse/lexer.hpp"
#include "aparse/parser.hpp"
#include "aparse/regex.hpp"
#include "aparse/regex_builder.hpp"
#include "aparse/parse_regex.hpp"

#include <tuple>

using std::string;
using std::unordered_map;
using aparse::ParserHelpers;



using std::cout;
using std::endl;

int main() {

  aparse::ParseRegex::Init();

  unordered_map<string, int> string_to_alphabet_map = {
                                    {"PLUS", 1},
                                    {"STAR", 2},
                                    {"NUMBER", 3},
                                    {"OPEN_B1", 4},
                                    {"CLOSE_B1", 5},
  };

  int alphabet_size, main_symbol;

  // auto lParse = [&](const string& input) {
  // };

  // lParse("main ::= expression* | TOKEN_STRING");
  // lParse("<main> ::= (<multiplied> PLUS)* <multiplied>");
  // lParse("<atom> ::= NUMBER | OPEN_B1 <main> CLOSE_B1 ");
  // lParse("<multiplied> ::= (<atoms> STAR)* <atom>");

  vector<string> rules = {
    "<main> ::= (<multiplied> PLUS)* <multiplied>",
    "<atom> ::= NUMBER | OPEN_B1 <main> CLOSE_B1 ",
    "<multiplied> ::= (<atom> STAR)* <atom>"
  };
  auto regex_rules = ParserHelpers::StringRulesToRegexRules(rules, string_to_alphabet_map, &alphabet_size, &main_symbol);
  for (auto& item: regex_rules) {
    cout << item.first << " : " << item.second.DebugString() << endl;
  }
  cout << "alphabet_size = " << alphabet_size << endl;
  cout << "main_symbol = " << main_symbol << endl;

//   auto regex = aparse::ParseRegex::Parse("([a-b]*|0)*(a|b)?$[0-9_]*");
// //  auto regex = aparse::ParseRegex::Parse("$[0-9_]*");
//   // auto regex = aparse::ParseRegex::Parse("b");
//   cout << "Done Parsing " << endl;
//   cout << regex.DebugString() << endl;

}

