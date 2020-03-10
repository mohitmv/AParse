// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_ABSTRACT_CORE_PARSER_HPP_
#define APARSE_ABSTRACT_CORE_PARSER_HPP_

#include <string>
#include <utility>
#include <vector>
#include <unordered_set>

#include <quick/utility.hpp>
#include <quick/debug_stream_decl.hpp>

#include "aparse/error.hpp"
#include "aparse/core_parse_node.hpp"

namespace aparse {

/** AParse can have support multiple different parsing mechanisms. Each
 *  mechanism must have a different
 *  (CoreParser, AParseMachine, AParseMachineBuilder) tuple. Interface of
 *  CoreParser must be derived from AbstractCoreParser.
 *
 *  AParseMachineBuilder :
 *    input:   AParseGrammar
 *    output:  AParseMachine.
 *    Details: AParseMachine should be built only once. Once built
 *             (or imported), it should live as long as we want parsing.
 *  CoreParser :
 *    input:  (const-reference of AParseMachine, string to parse)
 *    output: ParseTree.
 */
class AbstractCoreParser {
 public:
  AbstractCoreParser() = default;
  virtual ~AbstractCoreParser() = default;
  virtual void SetAParseMachine(const qk::AbstractType* machine) = 0;

  virtual bool Parse(CoreParseNode* output) = 0;
  virtual void ParseOrDie(CoreParseNode* output) = 0;
  virtual bool Parse(CoreParseNode* output, Error* error) = 0;

  virtual bool Feed(Alphabet alphabet) = 0;
  virtual void FeedOrDie(Alphabet alphabet) = 0;
  virtual bool Feed(Alphabet alphabet, Error* error) = 0;
  virtual bool Feed(const std::vector<Alphabet>& stream) = 0;
  virtual void FeedOrDie(const std::vector<Alphabet>& stream) = 0;
  virtual bool Feed(const std::vector<Alphabet>& stream, Error* error) = 0;

  virtual bool CanFeed(Alphabet alphabet) const = 0;
  virtual bool IsFinal() const = 0;
  virtual void Reset() = 0;
  virtual const std::vector<Alphabet>& GetStream() const = 0;
  virtual std::unordered_set<Alphabet> PossibleAlphabets() const = 0;
  virtual std::unordered_set<Alphabet> PossibleAlphabets(int k) const = 0;
};

}  // namespace aparse

#endif  // APARSE_ABSTRACT_CORE_PARSER_HPP_
