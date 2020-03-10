// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_INTERNAL_PARSER_BUILDER_HPP_
#define APARSE_INTERNAL_PARSER_BUILDER_HPP_

#include <tuple>
#include <memory>
#include <vector>
#include <string>

#include "quick/hash.hpp"
#include "quick/stl_utils.hpp"

#include "aparse/common_headers.hpp"
#include "aparse/error.hpp"
#include "aparse/aparse_grammar.hpp"
#include "aparse/parser_scope.hpp"
#include "aparse/parser.hpp"

#include <quick/utility.hpp>

namespace aparse {

// All the Build/Import methods are idempotent
class InternalParserBuilder {
 public:
  static void Build(const AParseGrammar& grammar,
             const vector<utils::any>& rule_actions,
             Parser* parser);

  static bool Import(const std::string& serialized_parser,
              std::size_t aparse_grammar_hash,
              const vector<utils::any>& rule_actions,
              Parser* parser);

  static void Export(const Parser& parser,
              std::size_t aparse_grammar_hash,
              std::string* serialized_parser);

  static std::string Export(const Parser& parser,
                     std::size_t aparse_grammar_hash);
};

}  // namespace aparse

#endif  // APARSE_INTERNAL_PARSER_BUILDER_HPP_
