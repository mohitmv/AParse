// Copyright: 2019 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <iostream>
#include <string>
#include <unordered_map>

#include "quick/debug.hpp"
#include "gtest/gtest.h"

#include "aparse/parser.hpp"
#include "aparse/parser_builder.hpp"

using std::string;
using std::unordered_map;

enum LexerScope : uint8_t {

  LITERAL,

        ANNOTATION_NAME,

        STRING_CONSTANT, INT_CONSTANT, DOUBLE_CONSTANT, BOOL_CONSTANT,

        OR_SYMBOL, ADD_SYMBOL, MINUS_SYMBOL, STAR_SYMBOL, DIVIDE_SYMBOL,
        EQUAL_SYMBOL, NOT_EQUAL_SYMBOL, LESS_SYMBOL, LESS_EQUAL_SYMBOL,
        GREATER_SYMBOL, GREATER_EQUAL_SYMBOL, POWER_SYMBOL,

        COMMA_SYMBOL, TRIPLE_DOT_SYMBOL, COLON_SYMBOL, SEMI_COLON,

        NULL_KEYWORD, TYPE_KEYWORD, OR_KEYWORD, AND_KEYWORD, NOT_KEYWORD,
        IN_KEYWORD, NOT_IN_KEYWORD,

        IF_KEYWORD, ELSE_KEYWORD, THEN_KEYWORD,

        TYPENAME_KEYWORD, TEMPLATE_KEYWORD, TYPEUNION_KEYWORD, ENUM_KEYWORD,
        OPERATOR_KEYWORD, RETURN_KEYWORD,

        OPTIONAL_KEYWORD, CONST_KEYWORD, REPEATED_KEYWORD,

        DATE_TIME_FORMAT_KEYWORD, TIME_FORMAT_KEYWORD, DATE_FORMAT_KEYWORD,
        YEAR_FORMAT_KEYWORD,

        NON_AGGREGATED_KEYWORD, GROUP_AGGREGATED_KEYWORD,
        AUTO_KEYWORD, AGGREGATED_KEYWORD,

        OPEN_B1, CLOSE_B1,
        OPEN_B2, CLOSE_B2};

static const unordered_map<string, LexerScope> kKeywords = {
  {"null", LexerScope::NULL_KEYWORD},
  {"|", LexerScope::OR_SYMBOL},
  {"or", LexerScope::OR_KEYWORD},
  {"and", LexerScope::AND_KEYWORD},
  {"+", LexerScope::ADD_SYMBOL},
  {"-", LexerScope::MINUS_SYMBOL},
  {"*", LexerScope::STAR_SYMBOL},
  {"/", LexerScope::DIVIDE_SYMBOL},
  {"^", LexerScope::POWER_SYMBOL},
  {"(", LexerScope::OPEN_B1},
  {")", LexerScope::CLOSE_B1},
  {"{", LexerScope::OPEN_B2},
  {"}", LexerScope::CLOSE_B2},
  {";", LexerScope::SEMI_COLON},
  {"=", LexerScope::EQUAL_SYMBOL},
  {"!=", LexerScope::NOT_EQUAL_SYMBOL},
  {"<=", LexerScope::LESS_EQUAL_SYMBOL},
  {"<", LexerScope::LESS_SYMBOL},
  {">", LexerScope::GREATER_SYMBOL},
  {">=", LexerScope::GREATER_EQUAL_SYMBOL},
  {",", LexerScope::COMMA_SYMBOL},
  {"...", LexerScope::TRIPLE_DOT_SYMBOL},
  {"return", LexerScope::RETURN_KEYWORD},
  {"in", LexerScope::IN_KEYWORD},
  {"not_in", LexerScope::NOT_IN_KEYWORD},
  {"if", LexerScope::IF_KEYWORD},
  {"else", LexerScope::ELSE_KEYWORD},
  {"then", LexerScope::THEN_KEYWORD},
  {":", LexerScope::COLON_SYMBOL},
  {"date_time_format_pattern", LexerScope::DATE_TIME_FORMAT_KEYWORD},
  {"time_format_pattern", LexerScope::TIME_FORMAT_KEYWORD},
  {"date_format_pattern", LexerScope::DATE_FORMAT_KEYWORD},
  {"year_format_pattern", LexerScope::YEAR_FORMAT_KEYWORD},
  {"optional", LexerScope::OPTIONAL_KEYWORD},
  {"const", LexerScope::CONST_KEYWORD},
  {"repeated", LexerScope::REPEATED_KEYWORD},
  {"non_aggregated", LexerScope::NON_AGGREGATED_KEYWORD},
  {"aggregated", LexerScope::AGGREGATED_KEYWORD},
  {"group_aggregated", LexerScope::GROUP_AGGREGATED_KEYWORD},
  {"auto", LexerScope::AUTO_KEYWORD},
  {"typename", LexerScope::TYPENAME_KEYWORD},
  {"template", LexerScope::TEMPLATE_KEYWORD},
  {"enum", LexerScope::ENUM_KEYWORD},
  {"typeunion", LexerScope::TYPEUNION_KEYWORD},
  {"operator", LexerScope::OPERATOR_KEYWORD},
  {"not", LexerScope::NOT_KEYWORD},
};

void BuildParser(aparse::Parser* parser, const std::string main_non_terminal = "main") {
  using Rule = aparse::ParserGrammar::Rule;
  aparse::ParserGrammar grammar;
  grammar.string_to_alphabet_map = {
            {"literal", LexerScope::LITERAL},
            {"string_constant", LexerScope::STRING_CONSTANT},
            {"int_constant", LexerScope::INT_CONSTANT},
            {"double_constant", LexerScope::DOUBLE_CONSTANT},
            {"bool_constant", LexerScope::BOOL_CONSTANT},
            {"type_keyword", LexerScope::TYPE_KEYWORD},
            {"annotation_name", LexerScope::ANNOTATION_NAME}
          };
  for (auto& item : kKeywords) {
    grammar.string_to_alphabet_map.emplace(item.first, item.second);
  }
  grammar.branching_alphabets = {{"if", "else"},
                                 {"(", ")"},
                                 {"{", "}"}};
  grammar.main_non_terminal = main_non_terminal;
  grammar.rules = {
    Rule("main ::= global_instruction*")
      ,
    Rule("global_instruction ::=   enum_declaration "
                                "| type_union_declaration "
                                "| signature_block "
                                "| global_variable_definition")
      ,
    Rule("literal_expr ::= literal")
      ,
    Rule("type_keyword_expr ::= type_keyword")
      ,
    Rule("type_union_expr ::= literal | type_keyword")
      ,
    Rule("string_constant_expr ::= string_constant")
      ,
    Rule("enum_declaration ::= 'enum' literal '{' ((literal_expr ',')* "
                                "literal_expr)? '}' ';'")
      ,
    Rule("type_union_declaration ::= 'typeunion' literal '=' "
                              "(type_keyword_expr '|')* type_keyword_expr ';'")
      ,
    Rule("const_expr ::=   int_constant | string_constant | literal "
                        "| double_constant | bool_constant")
      ,
    Rule("const_sage_expr ::=   int_constant | string_constant | literal "
                             "| null | double_constant | bool_constant")
      ,
    Rule("global_variable_definition ::= literal '=' const_expr ';'"),
    Rule("annotation ::= annotation_name '(' ((const_expr ',')* const_expr)? "
                                                                "')'")
      ,
    Rule("aggregation_specifiers ::=   'group_aggregated' "
                                    "| 'aggregated' "
                                    "| 'non_aggregated'")
      ,
    Rule("format_specifiers ::=    'date_time_format_pattern' "
                                "| 'time_format_pattern' "
                                "| 'year_format_pattern' "
                                "| 'date_format_pattern' ")
      ,
    Rule("op_expr ::=  'not' | '-' | '+' | '*' | '/' | '^' | 'and' | 'or' "
                     "| 'in' | '<' | '>' | '=' | '!=' | '<=' | '>=' | 'not_in'")
      ,
    Rule("declaration_input_arg_expr ::= aggregation_specifiers? "
                                        "('optional' | 'repeated')? 'const'? "
                                        "type_union_expr literal_expr?")
      ,
    Rule("declaration_input_args_expr ::= ((declaration_input_arg_expr ',')* "
                                          "declaration_input_arg_expr)?")
      ,
    Rule("io_declaration ::= format_specifiers? aggregation_specifiers? "
                             "type_union_expr "
                             "(literal_expr | ('operator' op_expr )) "
                             "'(' declaration_input_args_expr ')'")
      ,
    Rule("in_function_var_decl ::= literal '=' formula_expr ';'")
      ,
    Rule("signature_body ::= in_function_var_decl* 'return' formula_expr ';'")
      ,
    Rule("template_expr ::= 'template' '<' 'typename' literal ':' literal '>'")
      ,
    Rule("signature_block ::= annotation* "
                    "template_expr? "
                    "io_declaration+ (';' | '{' signature_body '}')")
      ,
    Rule("list_expr ::= EPSILON | (formula_expr ',')* formula_expr ")
      ,
    Rule("list_expr_atom ::= '{' list_expr '}' ")
      ,
    Rule("expr_function_atom ::= literal '(' list_expr ')'")
      ,
    Rule("formula_expr_atom_no_if ::=  const_sage_expr "
                            "| '(' formula_expr ')' "
                            "| list_expr_atom "
                            "| expr_function_atom")
      ,
    Rule("formula_expr_if_content ::= '(' formula_expr ')' 'then' formula_expr")
      ,
    Rule("formula_expr_atom ::= ('if' formula_expr_if_content 'else')* "
                                  "formula_expr_atom_no_if")
      ,
    Rule("expr_level_1 ::= ('-' | 'not')* formula_expr_atom")
      ,
    Rule("expr_level_3 ::= (expr_level_1 ('*'|'/'|'^'))* expr_level_1")
      ,
    Rule("expr_level_4 ::= (expr_level_3 ('+'|'-'))* expr_level_3")
      ,
    Rule("expr_level_5_1 ::= expr_level_4 ('<'|'>'|'<='|'>=') expr_level_4 ")
      ,
    Rule("expr_level_5_2 ::= (expr_level_4 ('='|'!='))* expr_level_4")
      ,
    Rule("expr_level_5 ::= expr_level_5_1 | expr_level_5_2")
      ,
    Rule("expr_level_6 ::= expr_level_5 (('in'|'not_in') expr_level_5)* ")
      ,
    // Rule("expr_level_7 ::= expr_level_6 ('not_in' expr_level_6)* ")
      // ,
    Rule("expr_level_7 ::= (expr_level_6 'and')* expr_level_6")
      ,
    Rule("formula_expr ::= (expr_level_7 'or')* expr_level_7")
      ,
  };
  aparse::ParserBuilder parser_builder;
  parser_builder.Build(grammar, parser);
}



TEST(InternalException, Basic) {
  aparse::Parser parser;
  BuildParser(&parser);
}
