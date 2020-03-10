// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

/** A test-utility, used for constructing sample AParseGrammar.
 *  The Alphabets in Regex as well as AParseGrammar are actually int32_t. Which
 *  is good for parsing efficiency but it makes difficult for a developer to
 *  understand the meaning of an alphabet. This utility allows to construct our
 *  grammar using string alphabets, which are later converted to integer
 *  alphabets by this utility. This utility generates the AParseGrammar as well
 *  as alphabet-to-string-map.
 */

#ifndef APARSE_SRC_SIMPLE_APARSE_GRAMMAR_BUILDER_HPP_
#define APARSE_SRC_SIMPLE_APARSE_GRAMMAR_BUILDER_HPP_

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <string>

#include "quick/byte_stream.hpp"
#include "quick/unordered_map.hpp"
#include "quick/unordered_set.hpp"
#include "quick/debug_stream_decl.hpp"
#include "quick/type_traits/function_type.hpp"

#include "aparse/regex.hpp"
#include "aparse/aparse_grammar.hpp"
#include "aparse/parser_scope.hpp"
#include "aparse/utils/any.hpp"
#include "src/utils.hpp"
#include "src/regex_builder.hpp"

namespace aparse {

namespace helpers {
void AParseGrammarIntAssignment(
    const vector<pair<string, RegexBuilderObject>>& string_grammar,
    const string& main_non_terminal,
    const vector<pair<string, string>>& branching_alphabets,
    AParseGrammar* grammar,
    std::unordered_map<string, int>* string_map);

}  // namespace helpers

class SimpleAParseGrammarBuilder {
 public:
  struct Rule {
    Rule() = default;
    Rule(const std::string& non_terminal,
         const RegexBuilderObject& rbo): non_terminal(non_terminal),
                                         regex_builder_object(rbo) {}
    template<typename T>
    Rule& Action(const T& action) {
      this->action = quick::function_type<T>(action);
      return *this;
    }
    std::string non_terminal;
    RegexBuilderObject regex_builder_object;
    utils::any action;
  };
  std::string main_non_terminal;
  std::vector<std::pair<string, string>> branching_alphabets;
  std::vector<Rule> rules;

  // idempotent ? : NO
  void Build() {
    vector<pair<string, RegexBuilderObject>> string_rules;
    for (auto& rule : rules) {
      string_rules.push_back(make_pair(rule.non_terminal,
                                       rule.regex_builder_object));
      rule_actions.push_back(rule.action);
    }
    helpers::AParseGrammarIntAssignment(string_rules,
                                        main_non_terminal,
                                        branching_alphabets,
                                        &this->aparse_grammar,
                                        &this->string_map);
  }
  vector<Alphabet> MakeStream(const vector<string>& string_stream) const {
    vector<Alphabet> output;
    for (auto& item : string_stream) {
      output.push_back(string_map.at(item));
    }
    return output;
  }
  AParseGrammar aparse_grammar;
  std::vector<utils::any> rule_actions;
  std::unordered_map<string, int> string_map;
};

}  // namespace aparse

#endif  // APARSE_SRC_SIMPLE_APARSE_GRAMMAR_BUILDER_HPP_
