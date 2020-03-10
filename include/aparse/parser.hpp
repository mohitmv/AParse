// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#ifndef APARSE_PARSER_HPP_
#define APARSE_PARSER_HPP_

#include <tuple>
#include <memory>
#include <vector>
#include <unordered_set>

#include "quick/hash.hpp"
#include "quick/stl_utils.hpp"

#include "aparse/common_headers.hpp"
#include "aparse/error.hpp"
#include "aparse/core_parse_node.hpp"
#include "aparse/parser_scope.hpp"
#include "aparse/syntax_tree_maker.hpp"

#include <quick/debug.hpp>
#include <quick/utility.hpp>

namespace aparse {

class AbstractCoreParser;

class Parser;

/** - To parse a string, client have to create ParserInstance object using
 *    Parser object.
 *  - ParserInstance can be used for feeding alphabets one by one. Parser object
 *    is created only once because Parser construction is a heavy computation.
 *  - Creation of ParserInstance from a const-reference of Parser object can
 *    be done everytime we have to parse a new string.
 *  - If the Parser object, from which ParserInstance is created, is
 *    destroyed, then ParserInstance will be invalidated. Hence Parser object
 *    must live longer than ParserInstance object.
 *  - Member of ParserInstance object are const-pointer
 *    to heavy lookup tables in Parser object.
 *  - Thread Safety:
 *     - Methods of ParserInstance are not thread safe. Locking wrapper around
 *       ParserInstance's method is not suggested. ParserInstance is a very
 *       light weight object.
 *     - For each thread create a new ParserInstance and work independently.
 *     - Parser Object (parent) must remain a constant object to allow
 *       concurrent creation of ParserInstances and support concurrent
 *       operations among different ParserInstance objects.
 *  - Learn more about ParserInstance at `https://aparse.readthedocs.io`. */
class ParserInstance {
 public:
  ParserInstance() = default;

  explicit ParserInstance(const Parser& parser);

  /** Initialize the ParserInstance object if it was default constructed.
   *  Is Idempotent : YES */
  void Init(const Parser& parser);

  /** Once a ParserInstance object is used for parsing a string, one can leave
   *  it and create another ParserInstance for parsing a new string. However a
   *  client can also choose to reuse the same ParserInstance object if they
   *  invoke Reset on it. The state of ParserInstance after Reset will be same
   *  as how it was just after creation. */
  void Reset();

  /** Feed a alphabet in the ParserInstance.
   *  @param a is the alphabet
   *  @return true if and only if Feed was successful, i.e. The string fed so
   *  far (F) is an acceptable prefix, i.e. There exists some string S s.t.
   *  `F + S` is an acceptable string. */
  bool Feed(Alphabet a);

  /** Same as Feed(Alphabet) with additional error details in case of
   *  unsuccessful feeding. Error detail includes: Alternative possible
   *  alphabets which could have fed, error position in the string etc.
   *  `error` must not be nullptr.
   *  Do `cout << error->what()` for detailed error message.
   *  Learn more at aparse::Error section in `https://aparse.readthedocs.io`. */
  bool Feed(Alphabet a, Error* error);

  /** Same as Feed(Alphabet, aparse::Error*), in case of unsuccessful feeding
   *  it throws the error object. (Note that aparse::Error is derived from
   *  std::exception) */
  void FeedOrDie(Alphabet a);

  /** It calls Feed(a) for each alphabet of string `s`. Returns as soon as
   *  first failure. Returns true in case of all the successful feeding. */
  bool Feed(const vector<Alphabet>& s);

  /** It calls Feed(a, error) for each alphabet of string `s`. Returns as soon
   *  as first failure with error details set in error pointer. Returns true in
   *  case of all the successful feeding. */
  bool Feed(const vector<Alphabet>& s, Error* error);

  /** It calls FeedOrDie(a) for each alphabet a of string `s`. */
  void FeedOrDie(const vector<Alphabet>& s);

  /** Once all the alphabets are fed, client have to call the End API.
   *  Returns true iff the string fed so far is an acceptable string. */
  bool End();

  /** Same as `End()` with additional error details in case of unsuccessful
   *  ending.
   *  `@error` must not be nullptr. */
  bool End(Error* error);

  /** In case of unsuccessful ending, it throws the error object.
   *  (Note that aparse::Error is derived from std::exception) */ 
  void EndOrDie();

  /** Returns true iff the string fed so far is an acceptable string.
   *  It's const method. It might influence client's decision to feed more
   *  alphabets or just call end. */
  bool IsFinal() const;

  /** At any point of time while feeding alphabet one by one, client can call
   *  this method to get the list of all the possible alphabets in that state.
   *  Let F is the acceptable-prefix fed so far. Then for each
   *  `x in PossibleAlphabets()`,  `F + x` is guaranteed to be
   *  acceptable-prefix. i.e. for each such x, there exists a string Y s.t.
   *  `F + x + Y` is an acceptable string.*/
  unordered_set<Alphabet> PossibleAlphabets() const;

  /** This method was designed to return only k possible alphabets. However
      currently it ignores the k and returns the output of
      `PossibleAlphabets()` */
  unordered_set<Alphabet> PossibleAlphabets(int k) const;


  /** Once End() is called after feeding all the alphabets one by one, client
   *  can use the CreateSyntaxTree method if RuleActions were provided in
   *  ParserGrammar. RuleActions are used in creation of syntax tree from the
   *  parse tree. If RuleActions are not provided, client can still get the
   *  ParseTree member of ParserInstance.
   *  Eg: `cout << parser_instance.parse_tree.DebugString()` */
  template<typename SyntaxTreeNode>
  void CreateSyntaxTree(SyntaxTreeNode* output) {
    ParserScopeBase<SyntaxTreeNode> scope;
    CreateSyntaxTree(&scope, output);
  }

  /** If the RuleActions require a custom ParserScope, it can be provided to
   *  CreateSyntaxTree after setting appropriate fields of custom ParserScope.
   *  Learn more about ParserGrammar construction at
   *  https://aparse.readthedocs.io */
  template<typename SyntaxTreeNode, typename ParserScope>
  void CreateSyntaxTree(ParserScope* scope, SyntaxTreeNode* output) {
    APARSE_ASSERT(parse_tree.IsInitialized());
    syntax_tree_maker->Build(parse_tree,
                             GetStream(),
                             scope,
                             output);
  }

 private:
  /** Returns the list of all the alphabets fed so far. This list is owned by
   *  core_parser, so this method returns const-reference. It's safe to use this
   *  const-reference as long as ParserInstance object is alive. */
  const vector<Alphabet>& GetStream() const;

  /** Parser and ParserInstance are shallow wrappers around CoreParser,
   *  exposing public facing fancy interfaces. Main work is done by either
   *  AParseMachineBuilder to build the AParseMachine for a given ParserGrammar
   *  or by CoreParser to parse a given string, using a const-reference of
   *  already built AParseMachine object
   *  ToDo(Mohit): Replace this shared_ptr by container_ptr :
   *               https://godbolt.org/z/TJhc8z  */
  std::shared_ptr<AbstractCoreParser> core_parser;

  /** ParseTree object. It will be constructed during the invocation of End()
   *  API, which needs to be called after feeding all alphabets */
  CoreParseNode parse_tree;
  const SyntaxTreeMaker* syntax_tree_maker;
};


/** The Parser object is the top level entity used for parsing strings.
 *  It is constructed using ParserBuilder::Build for a fixed ParserGrammar.
 *  Once constructed it can be used for parsing any number of times.*/
class Parser {
 public:
  /** More the compression -> smaller the size of AParseMachine -> smaller the
   *    AParseMachine build time -> higher the run time latency (Parsing).*/
  enum AParseMachineType : uint8_t {VERY_LOW_COMPRESSION,
                                    VERY_HIGH_COMPRESSION};

  /** To parse a string, client have to create ParserInstance object using this
   *  method. ParserInstance can be used for feeding alphabets one by one. */
  ParserInstance CreateInstance() const {
    return ParserInstance(*this);
  }

  /** Returns true iff Parser object is ready to parse strings. The variable
   * @is_finalized is set to true by `Finalize()` method which is called by
   * ParserBuilder at the final step of building this Parser. */
  inline bool IsFinalized () const { return is_finalized;}

 private:
  // ParserBuilder is granted permission to set private members of Parser.
  friend class ParserBuilder;
  friend class InternalParserBuilder;

  // ParserInstance is granted permission to read private members of Parser and
  // store the const-pointer of these objects.
  friend class ParserInstance;

  /** Finalize is used by ParserBuilder::Build after setting all the fields of
   *  Parser object. This method validates if Parser object is ready to parse.
   *  If everything is fine then it will set @is_finalized=true. */
  bool Finalize();

  /** Abstract pointer of AParseMachine object. In AParse we can have multiple
   *  different parsing mechanisms. A mechanisms might need their own
   *  representation of AParseMachine object. AParseMachine type must be
   *  derived from `quick::AbstractType` type.
   *  The `@machine_type` field controls the choice of AParseMachine.
   *  `src/v2/AParseMachine.hpp` is the AParseMachine used currently. */
  std::unique_ptr<quick::AbstractType> machine;

  /** @syntax_tree_maker is used for constructing SyntaxTree from a ParseTree.
   *  Learn more at `include/aparse/syntax_tree_maker.hpp`. */
  std::unique_ptr<SyntaxTreeMaker> syntax_tree_maker;

  /** Each rule of ParserGrammar can be attached with a mechanism of
   *  constructing syntax tree from a SubParseTree (A subtree of the ParseTree),
   *  whose root-node is the non-terminal of the current rule, assuming that
   *  syntax tree for the children of SubParseTree is already built and
   *  supplied as an input to discussed mechanism.
   *  Learn more about `RuleAction` at `https://aparse.readthedocs.io`. */
  std::vector<utils::any> rule_actions;
  std::vector<vector<Alphabet>> rule_atoms;
  std::vector<int> rule_non_terminals;
  AParseMachineType machine_type = AParseMachineType::VERY_HIGH_COMPRESSION;
  bool is_finalized = false;
};

}  // namespace aparse

#endif  // APARSE_PARSER_HPP_
