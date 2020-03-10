// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

// Single cpp file to compile and link AParse. To use aparse, users have to
// just include <aparse/aparse.hpp> and link their program with
// compilation unit `src/aparse.cpp`.

// Generated using:
// for i in $(ls src/*.cpp | cat | grep -v '_test'); do echo "#include \"$i\"" ; done #  NOLINT

#include "src/core_parse_node.cpp"  // NOLINT
#include "src/lexer.cpp"  // NOLINT
#include "src/internal_lexer_builder.cpp"  // NOLINT
#include "src/lexer_builder.cpp"  // NOLINT
#include "src/aparse_grammar.cpp"  // NOLINT
#include "src/error.cpp"  // NOLINT
#include "src/helpers.cpp"  // NOLINT
#include "src/internal_parser_builder.cpp"  // NOLINT
#include "src/lexer_machine_builder.cpp"  // NOLINT
#include "src/parse_char_regex.cpp"  // NOLINT
#include "src/parse_char_regex_rules.cpp"  // NOLINT
#include "src/parser_builder.cpp"  // NOLINT
#include "src/parser.cpp"  // NOLINT
#include "src/parse_regex_rule.cpp"  // NOLINT
#include "src/regex_builder.cpp"  // NOLINT
#include "src/regex.cpp"  // NOLINT
#include "src/regex_helpers.cpp"  // NOLINT
#include "src/simple_aparse_grammar_builder.cpp"  // NOLINT
#include "src/utils.cpp"  // NOLINT
#include "src/v2/aparse_machine_builder.cpp"  // NOLINT
#include "src/v2/aparse_machine.cpp"  // NOLINT
#include "src/v2/core_parser.cpp"  // NOLINT
#include "src/v2/internal_aparse_grammar.cpp"  // NOLINT
