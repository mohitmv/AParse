// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <utility>

#include "aparse/parser.hpp"
#include "src/simple_aparse_grammar_builder.hpp"
#include "quick/debug.hpp"


namespace test {
using aparse::AParseGrammar;
using aparse::Regex;
using std::string;
using std::vector;
using std::map;
using std::cout;
using std::endl;

struct Grammar1Node {
  std::vector<Grammar1Node> children;
  Grammar1Node() {}
  explicit Grammar1Node(std::vector<Grammar1Node>&& children)
    : children(std::move(children)) {}
  explicit Grammar1Node(const std::vector<Grammar1Node>& children)
    : children(children) {}
  string DebugString() const {
    std::ostringstream oss;
    std::function<void(const Grammar1Node&)> lDebugString;
    lDebugString = [&](const Grammar1Node& input) {
      for (auto& child : input.children) {
        oss << "(";
        lDebugString(child);
        oss << ")";
      }
    };
    lDebugString(*this);
    return oss.str();
  }
};

auto Grammar1() {
  using GrammarBuilder = aparse::SimpleAParseGrammarBuilder;
  using R = aparse::RegexBuilderObject;
  using ParserScope = aparse::ParserScopeBase<Grammar1Node>;
  GrammarBuilder grammar_builder;
  grammar_builder.branching_alphabets = {{"(", ")"}};
  grammar_builder.main_non_terminal = "main";
  // Rules:
  //   main ::= ( '(' main ')' )*
  grammar_builder.rules = {
    GrammarBuilder::Rule("main", (R("(") + R("main") + R(")")).Kstar())
      .Action([](ParserScope* scope, Grammar1Node* output) {
        output->children = std::move(scope->ValueList());
      })
  };
  grammar_builder.Build();
  return grammar_builder;
}

// Json Object
struct Grammar3Node {
  enum Type {LIST, MAP, INTEGER, STRING, BOOL_TYPE, NULL_TYPE};
  Type type = NULL_TYPE;
  bool bool_value;
  int int_value;
  string string_value;
  vector<Grammar3Node> list_value;
  map<string, Grammar3Node> map_value;
  string DebugString() const {
    std::ostringstream oss;
    std::function<void(const Grammar3Node&)> lDebugString;
    lDebugString = [&](const Grammar3Node& node) {
      switch (node.type) {
        case INTEGER:
          oss << "NUM";
          break;
        case STRING:
          oss << "STRING";
          break;
        case BOOL_TYPE:
          oss << "BOOL";
          break;
        case NULL_TYPE:
          oss << "NULL";
          break;
        case LIST: {
          oss << "[";
          bool is_first = true;
          for (auto& item : node.list_value) {
            oss << (is_first ? "": ", ");
            is_first = false;
            lDebugString(item);
          }
          oss << "]";
          break;
        }
        case MAP: {
          oss << "{";
          bool is_first = true;
          for (auto& item : node.map_value) {
            oss << (is_first ? "": ", ");
            is_first = false;
            oss << "STRING: ";
            lDebugString(item.second);
          }
          oss << "}";
          break;
        }
        default: assert(false);
      }
    };
    lDebugString(*this);
    return oss.str();
  }
};


auto Grammar3() {
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
  using GrammarBuilder = aparse::SimpleAParseGrammarBuilder;
  using ParserScope = aparse::ParserScopeBase<Grammar3Node>;
  using Rule = GrammarBuilder::Rule;
  using R = aparse::RegexBuilderObject;

  GrammarBuilder grammar_builder;
  grammar_builder.main_non_terminal = "Json";
  grammar_builder.branching_alphabets = {{"[", "]"}, {"{", "}"}};
  grammar_builder.rules = {
    Rule("Json", (   R("NUM")  // NOLINT
                  |  R("STRING")
                  |  R("BOOL")
                  |  R("NULL")
                  | (R("[") + R("JsonList") + R("]"))
                  | (R("{") + R("JsonDict") + R("}"))))
      .Action([](ParserScope* scope, Grammar3Node* output) {
        if (scope->Exists(0)) {
          output->type = Grammar3Node::INTEGER;
        } else if (scope->Exists(1)) {
          output->type = Grammar3Node::STRING;
        } else if (scope->Exists(2)) {
          output->type = Grammar3Node::BOOL_TYPE;
        } else if (scope->Exists(3)) {
          output->type = Grammar3Node::NULL_TYPE;
        } else {
          *output = std::move(scope->Value());
        }
      }),
    Rule("JsonList", ((R("Json") + R(",")).Kstar() + R("Json")).Optional())
      .Action([](ParserScope* scope, Grammar3Node* output) {
        output->type = Grammar3Node::LIST;
        output->list_value = std::move(scope->ValueList());
      }),
    Rule("JsonDict",
         ((R("JsonPair") + R(",")).Kstar() + R("JsonPair")).Optional())
      .Action([](ParserScope* scope, Grammar3Node* output) {
        output->type = Grammar3Node::MAP;
        for (auto& item : scope->ValueList()) {
          auto& item2 = *item.map_value.begin();
          output->map_value[std::move(item2.first)] = std::move(item2.second);
        }
      }),
    Rule("JsonPair", (R("STRING") + R(":") + R("Json")))
      .Action([](ParserScope* scope, Grammar3Node* output) {
        output->type = Grammar3Node::MAP;
        auto key = std::to_string(scope->AlphabetIndex());
        output->map_value[key] = std::move(scope->Value());
      })
  };
  grammar_builder.Build();
  return grammar_builder;
}

}  // namespace test
