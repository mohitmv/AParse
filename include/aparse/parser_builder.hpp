// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_PARSER_BUILDER_HPP_
#define APARSE_PARSER_BUILDER_HPP_

#include <tuple>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

#include <quick/type_traits/function_type.hpp>

#include "aparse/utils/very_common_headers.hpp"
#include "aparse/parser.hpp"
#include "src/internal_parser_builder.hpp"

namespace aparse {

/** ParserGrammar is used for defining the rule of parser grammar.
 *  Learn more at `src/parser_builder_integration_test.cpp`
 *  Learn more at https://aparse.readthedocs.io */
class ParserGrammar {
 public:
  struct Rule {
    explicit Rule(const std::string& rule): rule_string(rule) {}
    template<typename T>
    Rule& Action(const T& action) {
      this->action = quick::function_type<T>(action);
      return *this;
    }
    std::string rule_string;
    utils::any action;
  };

  std::vector<Rule> rules;
  string main_non_terminal;
  std::vector<std::pair<std::string, std::string>> branching_alphabets;
  std::unordered_map<string, Alphabet> string_to_alphabet_map;
};


class ParserBuilder {
 public:
  /** Once a Parser object is built, it can be exported to a file/string, so
   *  that it can be imported later. While importing the Parser object from a
   *  file/string, we still need ParserGrammar because RuleActions are C++
   *  function pointers / lambda objects / functors. These objects cannot be
   *  exported/imported. Rest of the stuff in Parser (heavy lookup tables) can
   *  be imported/export from byte string.
   *  Import(..) method will fail if the checksum of the given parser_grammar
   *  is different from the checksum of the original grammar which was
   *  exported. While exporting, checksum of grammar is also kept along with
   *  parser's heavy lookup tables. */
  static bool Import(const std::string& serialized_parser,
                     const ParserGrammar& parser_grammar,
                     Parser* parser);

  /** Export the Parser object into a string, which can be stored in a file. */
  static void Export(const Parser& parser,
                     const ParserGrammar& parser_grammar,
                     std::string* serialized_parser);

  /** Same as above Export method.
   * @returns serialized_parser. */
  static std::string Export(const Parser& parser,
                            const ParserGrammar& parser_grammar);

  /** Given a ParserGrammar, built the Parser object */
  static void Build(const ParserGrammar& parser_grammar, Parser* parser);
};

}  // namespace aparse

#endif  // APARSE_PARSER_BUILDER_HPP_
