// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <unordered_set>

#include <quick/stl_utils.hpp>

#include "src/parse_char_regex_rules.hpp"
#include "src/regex_helpers.hpp"

namespace aparse {

using uchar = unsigned char;

using helpers::C;
using helpers::U;
using helpers::A;
using helpers::KS;
using helpers::KP;
using helpers::E;
using helpers::SU;
using helpers::SC;

pair<AParseGrammar, vector<utils::any>> CharRegexParserRules() {
  using ParsingScope = CharRegexParserScope;
  AParseGrammar grammar;
  auto alphabet_size = grammar.alphabet_size = 256;
  auto ALL = helpers::AnyAlphabetRegex(grammar.alphabet_size);
  auto AE = [&](string s) {
    vector<Alphabet> clist;
    for (auto c : s) {
      clist.push_back(uchar(c));
    }
    return helpers::AllAlphabetExceptSomeRegex(clist, grammar.alphabet_size);
  };

  String2IntMapping s(grammar.alphabet_size);
  auto NT = [&](string str) {return Regex(s(str));};
  auto simple_char_set = AE("\\().+*|^-[]?");
  auto quoted_char_set = C({A('\\'), SU("\\().+*|^-[]?")});
  grammar.branching_alphabets = {{uchar('('), uchar(')')}};
  grammar.main_non_terminal = s("<main>");
  vector<utils::any> rule_actions;

  struct Rule {
    Rule(int nt, const Regex& r): non_terminal(nt), regex(r) {}

    Rule& Action(std::function<void(ParsingScope*, Regex*)> ra) {
      action = ra;
      return *this;
    }
    int non_terminal;
    Regex regex;
    utils::any action;
  };

  // Rules:
  // <simple_char_set> = simple_char_set;
  // <quoted_char_set> = quoted_char_set;
  // <atomic_char> ::= <simple_char_set> | <quoted_char_set>
  // <in_union_range_element> = <simple_char_set> "-" <simple_char_set>;
  // <in_union_element> ::= <in_union_range_element> | <atomic_char>;
  // <char_union> ::= "[" (""|"^") <in_union_element>* "]"
  // <atomic_expression> ::=  "(" <main> ")" | <atomic_char> | <char_union>
  // <main> ::= (<concat_expression> "|")* <concat_expression>
  // <unary_op_expression> ::= <atomic_expression> ("" | "+" | "*" | "?")
  // <concat_expression> ::= <unary_op_expression>+


  vector<Rule> rules = {
    Rule(s("<simple_char_set>"), simple_char_set)
      .Action([](ParsingScope* scope, Regex* output) {
        output->type = Regex::ATOMIC;
        output->alphabet = scope->GetAlphabet();
      }),

    Rule(s("<quoted_char_set>"), quoted_char_set)
      .Action([](ParsingScope* scope, Regex* output) {
        output->type = Regex::ATOMIC;
        output->alphabet = scope->content->at(scope->AlphabetIndex()+1);
      }),

    // <atomic_char> ::= <simple_char_set> | <quoted_char_set>
    Rule(s("<atomic_char>"),
         U({NT("<simple_char_set>"), NT("<quoted_char_set>")}))
      .Action([](ParsingScope* scope, Regex* output) {
        *output = std::move(scope->Value());
      }),

    // <in_union_range_element> = <simple_char_set> "-" <simple_char_set>;
    Rule(s("<in_union_range_element>"),
         C({NT("<simple_char_set>"), A('-'), NT("<simple_char_set>")}))
      .Action([](ParsingScope* scope, Regex* output) {
        output->type = Regex::UNION;
        int rs = scope->ValueList()[0].alphabet;
        int re = scope->ValueList()[1].alphabet;
        APARSE_DEBUG_ASSERT(rs <= re, "Invalid Range: (" << rs << ":"
                                                          << re << ")");
        for (int i = rs; i <= re; i++) {
          output->children.push_back(Regex(i));
        }
      }),

    // <in_union_element> ::= <in_union_range_element> | <atomic_char>;
    Rule(s("<in_union_element>"),
         U({NT("<in_union_range_element>"), NT("<atomic_char>")}))
      .Action([](ParsingScope* scope, Regex* output) {
        *output = std::move(scope->Value());
      }),

    // <char_union> ::= "[" (""|"^") <in_union_element>* "]"
    Rule(s("<char_union>"),
         C({A('['), U({E(), A('^')}), KS(NT("<in_union_element>")), A(']')}))
      .Action([alphabet_size](ParsingScope* scope, Regex* output) {
        if (scope->Exists(1)) {
          output->type = Regex::UNION;
          unordered_set<Alphabet> r_set;
          for (auto& item : scope->ValueList()) {
            r_set.insert(item.alphabet);
          }
          for (int i=0; i < alphabet_size; i++) {
            if (not qk::ContainsKey(r_set, i)) {
              output->children.push_back(Regex(i));
            }
          }
        } else {
          if (scope->ValueList().size() == 1) {
            *output = std::move(scope->ValueList()[0]);
          } else {
            output->type = Regex::UNION;
            output->children = std::move(scope->ValueList());
          }
        }
      }),

    // <atomic_expression> ::=  "(" <main> ")" | <atomic_char> | <char_union>
    Rule(s("<atomic_expression>"),
         U({C({A('('),
               NT("<main>"),
               A(')')}),
            NT("<atomic_char>"),
            NT("<char_union>")}))
      .Action([](ParsingScope* scope, Regex* output) {
        *output = std::move(scope->Value());
      }),

    // <main> ::= (<concat_expression> "|")* <concat_expression>
    Rule(s("<main>"),
         C({KS(C({NT("<concat_expression>"),
                  A('|')})),
            NT("<concat_expression>")}))
      .Action([](ParsingScope* scope, Regex* output) {
        if (scope->ValueList().size() == 1) {
          *output = std::move(scope->ValueList()[0]);
        } else {
          output->type = Regex::UNION;
          output->children = std::move(scope->ValueList());
        }
      }),

    // <unary_op_expression> ::= <atomic_expression> ("" | "+" | "*" | "?")
    Rule(s("<unary_op_expression>"),
         C({NT("<atomic_expression>"), U({E(), A('+'), A('*'), A('?')})}))
      .Action([](ParsingScope* scope, Regex* output) {
        if (scope->Exists(1)) {
          output->type = Regex::KPLUS;
          output->children.push_back(std::move(scope->Value()));
        } else if (scope->Exists(2)) {
          output->type = Regex::KSTAR;
          output->children.push_back(std::move(scope->Value()));
        } else if (scope->Exists(3)) {
          output->type = Regex::UNION;
          output->children.push_back(Regex(Regex::EPSILON));
          output->children.push_back(std::move(scope->Value()));
        } else {
          *output = std::move(scope->Value());
        }
      }),

    // <concat_expression> ::= <unary_op_expression>+
    Rule(s("<concat_expression>"),
         KS(NT("<unary_op_expression>")))
      .Action([](ParsingScope* scope, Regex* output) {
        if (scope->ValueList().size() > 1) {
          output->type = Regex::CONCAT;
          output->children = std::move(scope->ValueList());
        } else {
          *output = std::move(scope->Value());
        }
      })
  };
  for (auto& rule : rules) {
    grammar.rules.push_back(make_pair(rule.non_terminal, rule.regex));
    rule_actions.push_back(rule.action);
  }
  return make_pair(grammar, rule_actions);
}


}  // namespace aparse
