// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_ERROR_HPP_
#define APARSE_ERROR_HPP_

#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <sstream>

#include "aparse/common_headers.hpp"

namespace aparse {

/** aparse::Error is used for returning AParse specific errors/exceptions.
 * ToDo(Mohit): 1. Break down this class and create different derived classes
 *                 for the errors from different components.
 *              2. Construct the detailed error messages.
 *              3. This class has to be rewritten. */
struct Error: public std::exception {
  enum ErrorStatus {SUCCESS, INTERNAL_BUG,
                    LEXER_BUILDER_ERROR,
                    LEXER_BUILDER_ERROR_INVALID_REGEX,
                    LEXER_BUILDER_ERROR_MUST_HAVE_NON_ZERO_RULES,
                    LEXER_ERROR,
                    LEXER_ERROR_INVALID_TOKENS,
                    LEXER_ERROR_INCOMPLETE_TOKENS,

                    INTERNAL_GRAMMAR_REGEX_LABEL_MUST_BE_ZERO,
                    GRAMMAR_INVALID_USE_OF_BRANCHING_ALPHABET,
                    INTERNAL_GRAMMAR_MUST_HAVE_POSITIVE_ALPHABET_SIZE,
                    INTERNAL_GRAMMAR_MUST_HAVE_NON_ZERO_RULES,
                    INTERNAL_GRAMMAR_INVALID_NON_TERMINAL,
                    INTERNAL_GRAMMAR_INVALID_MAIN_NON_TERMINAL,
                    INTERNAL_GRAMMAR_UNDEFINED_NON_TERMINAL,
                    INTERNAL_GRAMMAR_INVALID_BRANCHING_ALPHABETS,
                    GRAMMAR_REPEATED_BRANCHING_ALPHABETS,
                    INTERNAL_GRAMMAR_NON_ENCLOSED_CYCLIC_DEPENDENCY,

                    GRAMMAR_DIRECT_COPY_RULES_ARE_NOT_SUPPORTED_YET,
                    GRAMMAR_REPEATED_NON_TERMINALS_ARE_NOT_SUPPORTED_YET,

                    PARSING_ERROR_INCOMPLETE_TOKENS,
                    PARSING_ERROR_INVALID_TOKENS,

                    PARSER_BUILDER_ERROR_INVALID_RULE,

                    INVALID_RULE_ACTION_TYPE,
                    INVALID_LEXER_RUN_ACTION_TYPE
                  };
  ErrorStatus status = SUCCESS;
  string error_message;
  std::string string_value;
  pair<int, int> error_position;
  unordered_set<Alphabet> possible_alphabets;
  Error() {}
  explicit Error(ErrorStatus status): status(status) {this->operator()();}

  Error& Status(ErrorStatus status) {
    this->status = status;
    this->operator()();
    return *this;
  }

  Error& StringValue(const string& s) {
    this->string_value = s;
    this->operator()();
    return *this;
  }

  Error(ErrorStatus status, string em): status(status), error_message(em) {
    this->operator()();
  }

  static string ErrorCodeString(ErrorStatus input);

  static string ErrorCodeMeaning(ErrorStatus input);

  bool Ok() {
    return (status == SUCCESS);
  }

  Error& PossibleAlphabets(const unordered_set<Alphabet>& possible_alphabets) {
    this->possible_alphabets = possible_alphabets;
    return *this;
  }

  Error& PossibleAlphabets(unordered_set<Alphabet>&& possible_alphabets) {
    this->possible_alphabets = std::move(possible_alphabets);
    return *this;
  }

  Error& Position(pair<int, int> error_position) {
    this->error_position = error_position;
    return *this;
  }

  Error& RuleNumber(int) {
    return *this;
  }

  Error& NonTerminal(int) {
    return *this;
  }

  Error& BranchingAlphabets(const pair<int, int>&) {
    return *this;
  }

  Error& operator()();
  virtual const char* what() const throw() {
    return error_message.c_str();
  }
};

}  // namespace aparse

#endif  // APARSE_ERROR_HPP_
