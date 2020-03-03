// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef _APARSE_INTERNAL_PARSER_BUILDER_HPP_
#define _APARSE_INTERNAL_PARSER_BUILDER_HPP_

#include <tuple>
#include <memory>

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
  void Build(const AParseGrammar& grammar,
             const vector<utils::any>& rule_actions,
             Parser* parser) const;

  bool Import(const std::string& serialized_parser,
              std::size_t aparse_grammar_hash,
              const vector<utils::any>& rule_actions,
              Parser* parser) const;

  void Export(const Parser& parser,
              std::size_t aparse_grammar_hash,
              std::string* serialized_parser) const;

  std::string Export(const Parser& parser,
                     std::size_t aparse_grammar_hash) const;

};

}  // namespace aparse

#endif  // _APARSE_INTERNAL_PARSER_BUILDER_HPP_
