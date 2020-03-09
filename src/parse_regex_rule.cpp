// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/parse_regex_rule.hpp"

#include <memory>
#include <algorithm>
#include <unordered_set>

#include "quick/stl_utils.hpp"
#include "src/regex_helpers.hpp"
#include "src/internal_parser_builder.hpp"
#include "src/internal_lexer_builder.hpp"

namespace aparse {

using std::unique_ptr;
using uchar = unsigned char;
using helpers::C;
using helpers::U;
using helpers::A;
using helpers::KS;
using helpers::KP;
using helpers::E;
using helpers::SU;
using helpers::SC;

struct GrammarRegexLexerScope: public aparse::LexerScopeBase {
  enum TokenType {NON_TERMINAL, LITERAL, PLUS_SYMBOL, STAR_SYMBOL,
                  QUESTION_MARK, OPEN_B1, STRING, QUESTION_MARK_SYMBOL,
                  CLOSE_B1, OPEN_B3, CLOSE_B3, GROUP_NEGATION_SYMBOL,
                  ASSIGNMENT_OP, OR_SYMBOL};
  using Token = pair<TokenType, pair<int, int>>;
  vector<Token> tokens;
};


struct RegexRuleParserScope: ParserScopeBase<ParsedGrammarRule> {
  using Token = GrammarRegexLexerScope::Token;
  using TokenType = GrammarRegexLexerScope::TokenType;
  const string* content;
  const vector<Token> *tokens;
  const unordered_map<std::string, TokenType> token_string_to_alphabet_mapping;
  string TokenString(int alphabet_index) const;
};

bool BuildGrammarRegexLexer(Lexer* lexer) {
  using Rule = aparse::InternalLexerGrammar::Rule;
  using Scope = GrammarRegexLexerScope;
  using TokenType = Scope::TokenType;
  aparse::InternalLexerGrammar lexer_rules;
  int alphabet_size = 256;
  auto ALL = helpers::AnyAlphabetRegex(alphabet_size);
  auto AE = [&](string s) {
    vector<Alphabet> clist;
    for (auto c : s) {
      clist.push_back(uchar(c));
    }
    return helpers::AllAlphabetExceptSomeRegex(clist, alphabet_size);
  };
  auto literal = ParseCharRegex::Parse("[a-zA-Z_][a-zA-Z0-9_]*");
  auto non_terminal = ParseCharRegex::Parse("<[a-zA-Z_][a-zA-Z0-9_]*>");
  auto string_regex_1 = C({A('"'),
                          KS(U({AE("\"\\"), C({A('\\'), ALL})})), A('"')});
  auto string_regex_2 = C({A('\''), KS(U({AE("\'\\"), C({A('\\'), ALL})})),
                          A('\'')});
  auto string_regex = U({string_regex_1, string_regex_2});
  lexer_rules.main_section = 0;
  auto lAddTokenAction = [](TokenType type) {
    return [type](Scope* scope) {
              scope->tokens.push_back(make_pair(type, scope->Range()));
            };
  };
  lexer_rules.rules = {
    {0, {
      Rule(literal).Action(lAddTokenAction(Scope::LITERAL)),
      Rule(non_terminal).Action(lAddTokenAction(Scope::NON_TERMINAL)),
      Rule(string_regex).Action(lAddTokenAction(Scope::STRING)),
      Rule(KP(U({A(' '), A('\t'), A('\n')}))),
    }}
  };
  for (auto& kv : unordered_map<string, TokenType> {
                {"(", Scope::OPEN_B1},
                {")", Scope::CLOSE_B1},
                {"[", Scope::OPEN_B3},
                {"]", Scope::CLOSE_B3},
                {"+", Scope::PLUS_SYMBOL},
                {"*", Scope::STAR_SYMBOL},
                {"?", Scope::QUESTION_MARK_SYMBOL },
                {"^", Scope::GROUP_NEGATION_SYMBOL},
                {"::=", Scope::ASSIGNMENT_OP},
                {"|", Scope::OR_SYMBOL},
                {"?", Scope::QUESTION_MARK_SYMBOL},
              }) {
    lexer_rules.rules.at(0).push_back(
      Rule(SC(kv.first)).Action(lAddTokenAction(kv.second)));
  }
  return InternalLexerBuilder::Build(lexer_rules, lexer);
}


bool BuildGrammarRegexParser(Parser* parser) {
  using ParserScope = RegexRuleParserScope;
  using LexerScope = GrammarRegexLexerScope;
  using TokenType = LexerScope::TokenType;
  using RuleActionType = std::function<void(ParserScope*, ParsedGrammarRule*)>;
  AParseGrammar grammar;
  grammar.alphabet_size = 50;
  grammar.branching_alphabets = {{LexerScope::OPEN_B1, LexerScope::CLOSE_B1}};
  String2IntMapping s(grammar.alphabet_size);
  grammar.main_non_terminal = s("<main>");
  auto A = [&](Alphabet a) {return Regex(a);};
  auto NT = [&](const string& str) {return Regex(s(str));};

  struct Rule {
    Rule(int nt, const Regex& r): non_terminal(nt), regex(r) {}
    Rule& Action(RuleActionType ra) {
      action = ra;
      return *this;
    }
    int non_terminal;
    Regex regex;
    utils::any action;
  };

  // Rules:
  // <main> ::= (NON_TERMINAL|LITERAL) "::=" <main_expression>
  // <atoms> ::= LITERAL | NON_TERMINAL | STRING
  // <alphabet_union> ::= "[" (""|"^") <atoms>* "]"
  // <atomic_expression> ::=  "(" <main_expression> ")" | <atoms>
  // <main_expression> ::= (<concat_expression> "|")* <concat_expression>
  // <unary_op_expression> ::= <atomic_expression> ("" | "+" | "*" | "?")
  // <concat_expression> ::= <unary_op_expression>+

  vector<Rule> rules = {
    Rule(s("<main>"),
          C({U({A(LexerScope::NON_TERMINAL), A(LexerScope::LITERAL)}),
             A(LexerScope::ASSIGNMENT_OP),
             NT("<main_expression>")}))
      .Action([](ParserScope* scope, ParsedGrammarRule* output) {
        output->type = ParsedGrammarRule::RULE;
        output->value = scope->TokenString(scope->AlphabetIndexList()[0]);
        output->children.push_back(std::move(scope->Value()));
      }),
    Rule(s("<atoms>"),
          U({A(LexerScope::LITERAL),
             A(LexerScope::NON_TERMINAL),
             A(LexerScope::STRING)}))
      .Action([](ParserScope* scope, ParsedGrammarRule* output) {
        if (scope->Exists(0)) {
          string value = scope->TokenString(scope->AlphabetIndex());
          if (value == "EPSILON") {
            output->type = ParsedGrammarRule::EPSILON;
          } else {
            output->type = ParsedGrammarRule::LITERAL;
            output->value = value;
          }
        } else if (scope->Exists(1)) {
          output->type = ParsedGrammarRule::NON_TERMINAL;
          output->value = scope->TokenString(scope->AlphabetIndex());
        } else if (scope->Exists(2)) {
          output->type = ParsedGrammarRule::STRING;
          output->value = scope->TokenString(scope->AlphabetIndex());
          output->value = output->value.substr(1, output->value.size()-2);
        } else {
          assert(false);
        }
      }),

    // <atomic_expression> ::=  "(" <main_expression> ")" | <atoms>
    Rule(s("<atomic_expression>"),
          U({NT("<atoms>"), C({A(LexerScope::OPEN_B1),
                               NT("<main_expression>"),
                               A(LexerScope::CLOSE_B1)})}))
      .Action([](ParserScope* scope, ParsedGrammarRule* output) {
        *output = std::move(scope->Value());
      }),

    // <main_expression> ::= (<concat_expression> "|")* <concat_expression>
    Rule(s("<main_expression>"),
          C({KS(C({NT("<concat_expression>"),
                   A(LexerScope::OR_SYMBOL)})), NT("<concat_expression>")}))
      .Action([](ParserScope* scope, ParsedGrammarRule* output) {
        if (scope->ValueList().size() == 1) {
          *output = std::move(scope->ValueList()[0]);
        } else {
          output->type = ParsedGrammarRule::UNION;
          output->children = std::move(scope->ValueList());
        }
      }),

    // <unary_op_expression> ::= <atomic_expression> ("" | "+" | "*" | "?")
    Rule(s("<unary_op_expression>"),
          C({NT("<atomic_expression>"),
              U({E(),
                 A(LexerScope::PLUS_SYMBOL),
                 A(LexerScope::STAR_SYMBOL),
                 A(LexerScope::QUESTION_MARK_SYMBOL)})}))
      .Action([](ParserScope* scope, ParsedGrammarRule* output) {
        if (scope->Exists(1)) {
          output->type = ParsedGrammarRule::KPLUS;
          output->children.push_back(std::move(scope->Value()));
        } else if (scope->Exists(2)) {
          output->type = ParsedGrammarRule::KSTAR;
          output->children.push_back(std::move(scope->Value()));
        } else if (scope->Exists(3)) {
          output->type = ParsedGrammarRule::UNION;
          output->children.push_back(
              ParsedGrammarRule(ParsedGrammarRule::EPSILON));
          output->children.push_back(std::move(scope->Value()));
        } else {
          *output = std::move(scope->Value());
        }
      }),

    // <concat_expression> ::= <unary_op_expression>+
    Rule(s("<concat_expression>"),
          KS(NT("<unary_op_expression>")))
      .Action([](ParserScope* scope, ParsedGrammarRule* output) {
        if (scope->ValueList().size() > 1) {
          output->type = ParsedGrammarRule::CONCAT;
          output->children = std::move(scope->ValueList());
        } else {
          *output = std::move(scope->Value());
        }
      }),

    // <alphabet_union> ::= "[" (""|"^") <atoms>* "]"
    Rule(s("<alphabet_union>"),
      C({A(LexerScope::OPEN_B3),
         U({E(), A(LexerScope::GROUP_NEGATION_SYMBOL)}),
         KS(NT("<atoms>")),
         A(LexerScope::CLOSE_B3)}))
      .Action([](ParserScope* scope, ParsedGrammarRule* output) {
        if (scope->Exists(1)) {
          output->type = ParsedGrammarRule::ALL_EXCEPT;
          output->children = std::move(scope->ValueList());
        } else {
          if (scope->ValueList().size() == 1) {
            *output = std::move(scope->ValueList()[0]);
          } else {
            output->type = ParsedGrammarRule::UNION;
            output->children = std::move(scope->ValueList());
          }
        }
      }),
  };
  vector<utils::any> rule_actions;
  for (auto& rule : rules) {
    grammar.rules.push_back(make_pair(rule.non_terminal, rule.regex));
    rule_actions.push_back(rule.action);
  }
  {
    InternalParserBuilder::Build(grammar, rule_actions, parser);
  }
  return true;
}

ParsedGrammarRule ParseRegexRule::Parse(const string& input) {
  ParsedGrammarRule output;
  Error error;
  if (not Parse(input, &output, &error)) {
    throw error;
  }
  return output;
}

bool ParseRegexRule::Parse(const string& input,
                           ParsedGrammarRule* output,
                           Error* error) {
  static Parser grammar_regex_parser;
  static Lexer grammar_regex_lexer;
  if (not grammar_regex_lexer.IsInitialized()) {
    BuildGrammarRegexLexer(&grammar_regex_lexer);
  }
  if (not grammar_regex_parser.IsFinalized()) {
    BuildGrammarRegexParser(&grammar_regex_parser);
  }
  GrammarRegexLexerScope lexer_scope;
  auto lexer = grammar_regex_lexer.CreateInstance(&lexer_scope);
  auto parser = grammar_regex_parser.CreateInstance();
  lexer.Reset();
  for (char c : input) {
    if (not lexer.Feed(uchar(c), error)) {
      return false;
    }
  }
  if (not lexer.End(error)) {
    return false;
  }
  auto& tokens = lexer_scope.tokens;
  parser.Reset();
  for (auto& token : tokens) {
    if (not parser.Feed(token.first, error)) {
      return false;
    }
  }
  if (not parser.End(error)) {
    return false;
  }
  RegexRuleParserScope scope;
  scope.content = &input;
  scope.tokens = &tokens;
  parser.CreateSyntaxTree(&scope, output);
  return true;
}

bool ParsedGrammarRule::operator==(const ParsedGrammarRule& other) const {
  return (type == other.type
          && value == other.value
          && children == other.children);
}

string ParsedGrammarRule::DebugString() const {
  std::ostringstream oss;
  std::function<void(const ParsedGrammarRule& input)> lPrint;
  lPrint = [&](const ParsedGrammarRule& input) {
    oss << "type[" << static_cast<int>(input.type) << "], ";
    oss << "value[" << input.value << "], ";
    for (int i = 0; i < input.children.size(); i++) {
      oss << "children[" << i << "] = {";
      lPrint(input.children[i]);
      oss << "}, ";
    }
  };
  lPrint(*this);
  return oss.str();
}

string RegexRuleParserScope::TokenString(int alphabet_index) const {
  auto range = tokens->at(alphabet_index).second;
  return content->substr(range.first, range.second - range.first);
}

namespace helpers {

void ExtractRuleTerminals(const ParsedGrammarRule& input,
                          unordered_set<string>* output) {
  switch (input.type) {
    case ParsedGrammarRule::LITERAL:
    case ParsedGrammarRule::NON_TERMINAL:
    case ParsedGrammarRule::STRING:
      output->insert(input.value);
      break;
    case ParsedGrammarRule::UNION:
    case ParsedGrammarRule::CONCAT:
    case ParsedGrammarRule::KPLUS:
    case ParsedGrammarRule::KSTAR:
    case ParsedGrammarRule::ALL_EXCEPT:
      for (auto& item : input.children) {
        ExtractRuleTerminals(item, output);
      }
      break;
    case ParsedGrammarRule::EPSILON:
      break;
    default:
      assert(false);
  }
}

void BuildRegex(const ParsedGrammarRule& input,
                const unordered_map<string, int>& string_map,
                int alphabet_size,
                Regex* output) {
  switch (input.type) {
    case ParsedGrammarRule::LITERAL:
    case ParsedGrammarRule::NON_TERMINAL:
    case ParsedGrammarRule::STRING:
      output->type = Regex::ATOMIC;
      output->alphabet = string_map.at(input.value);
      break;
    case ParsedGrammarRule::UNION:
    case ParsedGrammarRule::CONCAT:
    case ParsedGrammarRule::KPLUS:
    case ParsedGrammarRule::KSTAR: {
      switch (input.type) {
        case ParsedGrammarRule::UNION:
          output->type = Regex::UNION;
          break;
        case ParsedGrammarRule::CONCAT:
          output->type = Regex::CONCAT;
          break;
        case ParsedGrammarRule::KPLUS:
          output->type = Regex::KPLUS;
          break;
        case ParsedGrammarRule::KSTAR:
          output->type = Regex::KSTAR;
          break;
        default: assert(false);
      }
      output->children.resize(input.children.size());
      for (int i = 0; i < input.children.size(); i++) {
        BuildRegex(input.children[i],
                   string_map,
                   alphabet_size,
                   &output->children[i]);
      }
      break;
    }
    case ParsedGrammarRule::EPSILON:
      output->type = Regex::EPSILON;
      break;
    case ParsedGrammarRule::ALL_EXCEPT: {
      output->type = Regex::UNION;
      unordered_set<int> excluded_values;
      for (auto& item : input.children) {
        excluded_values.insert(string_map.at(item.value));
      }
      for (int i = 0; i < alphabet_size; i++) {
        if (not qk::ContainsKey(excluded_values, i)) {
          output->children.push_back(Regex(i));
        }
      }
      break;
    }
    default:
      assert(false);
  }
}

}  // namespace helpers


AParseGrammar helpers::StringRulesToAParseGrammar(
    const vector<string>& rule_strings,
    const unordered_map<string, Alphabet>& string_to_alphabet_map,
    const vector<pair<string, string>>& branching_alphabets,
    const string& main_non_terminal) {
  AParseGrammar grammar;
  vector<ParsedGrammarRule> rules;
  unordered_map<string, int> non_terminals;
  unordered_map<string, int> terminals;
  std::function<void(const ParsedGrammarRule&)> lExtractTerminals;
  lExtractTerminals = [&](const ParsedGrammarRule& input) {
    switch (input.type) {
      case ParsedGrammarRule::LITERAL:
      case ParsedGrammarRule::NON_TERMINAL:
      case ParsedGrammarRule::STRING:
        terminals[input.value];
        break;
      case ParsedGrammarRule::UNION:
      case ParsedGrammarRule::CONCAT:
      case ParsedGrammarRule::KPLUS:
      case ParsedGrammarRule::KSTAR:
      case ParsedGrammarRule::ALL_EXCEPT:
        for (auto& item : input.children) {
          lExtractTerminals(item);
        }
        break;
      case ParsedGrammarRule::EPSILON:
        break;
      default:
        assert(false);
    }
  };
  Error error;
  for (auto& rule_string : rule_strings) {
    ParsedGrammarRule rule;
    if (not ParseRegexRule::Parse(rule_string, &rule, &error)) {
      error.Status(Error::PARSER_BUILDER_ERROR_INVALID_RULE)
           .StringValue(string("Rule: ") + rule_string + "\n");
      throw error;
    }
    APARSE_DEBUG_ASSERT(rule.type == ParsedGrammarRule::RULE);
    non_terminals[rule.value];
    lExtractTerminals(rule.children[0]);
    rules.push_back(std::move(rule));
  }
  auto string_map = string_to_alphabet_map;
  if (string_map.size() == 0) {
    int counter = 0;
    for (auto& item : terminals) {
      auto& s = item.first;
      if (not qk::ContainsKey(non_terminals, s) and
          not qk::ContainsKey(string_map, s)) {
        string_map[s] = counter++;
      }
    }
  }
  int max_alphabet = 0;
  for (auto& item : string_map) {
    max_alphabet = std::max(max_alphabet, item.second);
    APARSE_DEBUG_ASSERT(not qk::ContainsKey(non_terminals, item.first));
  }
  int index = max_alphabet+1;
  for (auto& item : non_terminals) {
    item.second = ++index;
  }
  for (auto& item : terminals) {
    if (qk::ContainsKey(string_map, item.first)) {
      item.second = string_map.at(item.first);
    } else {
      APARSE_DEBUG_ASSERT(qk::ContainsKey(non_terminals, item.first),
                           item.first);
      item.second = non_terminals[item.first];
    }
  }
  std::function<void(const ParsedGrammarRule&, Regex*)> lBuildRegex;
  lBuildRegex = [&](const ParsedGrammarRule& input, Regex* output) {
    switch (input.type) {
      case ParsedGrammarRule::LITERAL:
      case ParsedGrammarRule::NON_TERMINAL:
      case ParsedGrammarRule::STRING:
        output->type = Regex::ATOMIC;
        output->alphabet = terminals.at(input.value);
        break;
      case ParsedGrammarRule::UNION:
      case ParsedGrammarRule::CONCAT:
      case ParsedGrammarRule::KPLUS:
      case ParsedGrammarRule::KSTAR: {
        switch (input.type) {
          case ParsedGrammarRule::UNION:
            output->type = Regex::UNION;
            break;
          case ParsedGrammarRule::CONCAT:
            output->type = Regex::CONCAT;
            break;
          case ParsedGrammarRule::KPLUS:
            output->type = Regex::KPLUS;
            break;
          case ParsedGrammarRule::KSTAR:
            output->type = Regex::KSTAR;
            break;
          default: assert(false);
        }
        output->children.resize(input.children.size());
        for (int i = 0; i < input.children.size(); i++) {
          lBuildRegex(input.children[i], &output->children[i]);
        }
        break;
      }
      case ParsedGrammarRule::EPSILON:
        output->type = Regex::EPSILON;
        break;
      case ParsedGrammarRule::ALL_EXCEPT: {
        output->type = Regex::UNION;
        unordered_set<int> excluded_values;
        for (auto& item : input.children) {
          excluded_values.insert(terminals.at(item.value));
        }
        for (int i = 0; i <= max_alphabet; i++) {
          if (not qk::ContainsKey(excluded_values, i)) {
            output->children.push_back(Regex(i));
          }
        }
        break;
      }
      default:
        assert(false);
    }
  };
  for (auto& rule : rules) {
    Regex regex;
    lBuildRegex(rule.children[0], &regex);
    grammar.rules.push_back(make_pair(non_terminals.at(rule.value), regex));
  }
  grammar.alphabet_size = 1 + max_alphabet;
  for (auto& item : branching_alphabets) {
    APARSE_DEBUG_ASSERT(qk::ContainsKey(string_map, item.first));
    APARSE_DEBUG_ASSERT(qk::ContainsKey(string_map, item.second));
    grammar.branching_alphabets.push_back(
        make_pair(string_map.at(item.first),
                  string_map.at(item.second)));
  }
  APARSE_DEBUG_ASSERT(qk::ContainsKey(non_terminals, main_non_terminal));
  grammar.main_non_terminal = non_terminals.at(main_non_terminal);
  return grammar;
}

}  // namespace aparse
