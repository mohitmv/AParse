// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <sstream>
#include <functional>
#include <algorithm>

#include "quick/debug.hpp"
#include "quick/stl_utils.hpp"

#include "aparse/aparse_grammar.hpp"
#include "aparse/error.hpp"
#include "quick/hash.hpp"

#include "src/regex_helpers.hpp"
#include "src/helpers.hpp"

namespace aparse {
using std::string;
using std::unordered_set;

namespace {

// This method is deprecated. Use utils::TopologicalSortingInGraph instead.
// @graph is a map from node -> set of outgoing nodes.
// returns true if there is no cycle.
bool CycleDetectionInGraph(std::unordered_map<int,
                                              unordered_set<int>> graph) {
  std::unordered_set<int> visited;
  std::unordered_set<int> tested;
  // returns true if there is no cycle.
  std::function<bool(int)> lDFA = [&](int u) {
    visited.insert(u);
    for (auto& oe : graph.at(u)) {
      if ((!qk::ContainsKey(tested, oe)) &&
          (qk::ContainsKey(visited, oe) || !lDFA(oe))) {
        return false;
      }
    }
    visited.erase(u);
    tested.insert(u);
    return true;
  };
  for (auto& item : graph) {
    if (!qk::ContainsKey(tested, item.first) && !lDFA(item.first)) {
      return false;
    }
  }
  return true;
}

// 1. Validates the proper use of branching alphabets.
// 2. Validates that ragex.label should be zero for all regex.
// 3. Extracts the directly required non-terminals.
bool ProcessRuleRegex(const Regex& regex,
                      const std::unordered_map<int, int>& ba_map,
                      const std::unordered_map<int, int>& ba_inverse_map,
                      int rule_number,
                      int alphabet_size,
                      std::unordered_set<int>* dependent_non_terminals) {
  std::function<void(const Regex& regex)> lValidateRuleRegex;
  auto lIsBAStart = [&](const Regex& r) {
    return (r.type == Regex::ATOMIC && qk::ContainsKey(ba_map, r.alphabet));
  };
  auto lIsBAEnd = [&](const Regex& r) {
    return (r.type == Regex::ATOMIC
              && qk::ContainsKey(ba_inverse_map, r.alphabet));
  };
  lValidateRuleRegex = [&](const Regex& regex) {
    if (regex.label != 0) {
      throw Error(Error::INTERNAL_GRAMMAR_REGEX_LABEL_MUST_BE_ZERO);
    }
    switch (regex.type) {
      case Regex::ATOMIC:
      case Regex::EPSILON: break;
      case Regex::UNION:
      case Regex::KPLUS:
      case Regex::ID:
      case Regex::KSTAR:
        for (auto& child : regex.children) {
          lValidateRuleRegex(child);
        }
        break;
      case Regex::CONCAT: {
        vector<int> stack;
        for (auto& child : regex.children) {
          if (lIsBAStart(child)) {
            stack.push_back(child.alphabet);
          } else if (lIsBAEnd(child)) {
            if (stack.size() > 0
                  && stack.back() == ba_inverse_map.at(child.alphabet)) {
              stack.pop_back();
            } else {
              throw Error(Error::GRAMMAR_INVALID_USE_OF_BRANCHING_ALPHABET)
                      .RuleNumber(rule_number)();
            }
          }
        }
        if (stack.size() > 0) {
          throw Error(Error::GRAMMAR_INVALID_USE_OF_BRANCHING_ALPHABET)
                  .RuleNumber(rule_number)();
        }
        for (auto& child : regex.children) {
          lValidateRuleRegex(child);
        }
        break;
      }
    }
  };
  std::function<void(const Regex& regex)> lExtractDependentNonTerminals;
  lExtractDependentNonTerminals = [&](const Regex& regex) {
    switch (regex.type) {
      case Regex::EPSILON:
        break;
      case Regex::ATOMIC:
        if (regex.alphabet >= alphabet_size) {
          dependent_non_terminals->insert(regex.alphabet);
        }
        break;
      case Regex::UNION:
      case Regex::KPLUS:
      case Regex::ID:
      case Regex::KSTAR:
        for (auto& child : regex.children) {
          lExtractDependentNonTerminals(child);
        }
        break;
      case Regex::CONCAT: {
        int stack_depth = 0;
        for (auto& child : regex.children) {
          if (lIsBAStart(child)) {
            stack_depth++;
          } else if (lIsBAEnd(child)) {
            stack_depth--;
          } else if (stack_depth == 0) {
            lExtractDependentNonTerminals(child);
          }
        }
        break;
      }
    }
  };
  lValidateRuleRegex(regex);
  lExtractDependentNonTerminals(regex);
  return true;
}

}  // namespace


bool AParseGrammar::Validate() const {
  if (alphabet_size <= 0) {
    throw Error(Error::INTERNAL_GRAMMAR_MUST_HAVE_POSITIVE_ALPHABET_SIZE)();
  } else if (rules.size() == 0) {
    throw Error(Error::INTERNAL_GRAMMAR_MUST_HAVE_NON_ZERO_RULES)();
  } else {
    std::unordered_set<int> non_terminals;
    for (int i = 0; i < rules.size(); i++) {
      int nt = rules[i].first;
      if (nt < alphabet_size) {
        throw Error(Error::INTERNAL_GRAMMAR_INVALID_NON_TERMINAL)
            .NonTerminal(nt).RuleNumber(i)();
      }
      non_terminals.insert(rules[i].first);
    }
    if (not qk::ContainsKey(non_terminals, main_non_terminal)) {
      throw Error(Error::INTERNAL_GRAMMAR_INVALID_MAIN_NON_TERMINAL)();
    }
    if (non_terminals.size() != rules.size()) {
      throw Error(Error::GRAMMAR_REPEATED_NON_TERMINALS_ARE_NOT_SUPPORTED_YET);
    }
    std::unordered_set<int> alphabets;
    for (int i = 0; i < rules.size(); i++) {
      auto atoms = helpers::GetRegexAtoms(rules[i].second);
      for (auto& atom : atoms) {
        if (0 <= atom && atom < alphabet_size) {
          alphabets.insert(atom);
        } else if (not qk::ContainsKey(non_terminals, atom)) {
          throw Error(Error::INTERNAL_GRAMMAR_UNDEFINED_NON_TERMINAL)
                  .NonTerminal(atom).RuleNumber(i)();
        }
      }
    }
    std::unordered_set<int> branching_alphabets_set;
    std::unordered_map<int, int> ba_map;
    std::unordered_map<int, int> ba_inverse_map;
    for (auto& item : branching_alphabets) {
      branching_alphabets_set.insert(item.first);
      branching_alphabets_set.insert(item.second);
      ba_map[item.first] = item.second;
      ba_inverse_map[item.second] = item.first;
      if (not (qk::ContainsKey(alphabets, item.first)
                && qk::ContainsKey(alphabets, item.second))) {
        throw Error(Error::INTERNAL_GRAMMAR_INVALID_BRANCHING_ALPHABETS)
                .BranchingAlphabets(item)();
      }
    }
    if (branching_alphabets_set.size() < branching_alphabets.size()*2) {
      throw Error(Error::GRAMMAR_REPEATED_BRANCHING_ALPHABETS)();
    }
    std::unordered_map<int, std::unordered_set<int>> dependency_graph;
    for (int i = 0; i < rules.size(); i++) {
      const Regex& regex = rules[i].second;
      if (regex.type == Regex::ATOMIC
            && qk::ContainsKey(non_terminals, regex.alphabet)) {
        throw Error(Error::GRAMMAR_DIRECT_COPY_RULES_ARE_NOT_SUPPORTED_YET)();
      }
      ProcessRuleRegex(regex,
                       ba_map,
                       ba_inverse_map,
                       i,
                       alphabet_size,
                       &dependency_graph[rules[i].first]);
    }
    if (not CycleDetectionInGraph(dependency_graph)) {
      throw Error(Error::INTERNAL_GRAMMAR_NON_ENCLOSED_CYCLIC_DEPENDENCY)();
    }
  }
  return true;
}

string AParseGrammar::DebugString() const {
  bool has_alphabet_map = (alphabet_map.size() > 0);
  std::ostringstream oss;
  oss << "{\n";
  oss << "  Alphabet Size = " << alphabet_size << "\n";
  if (has_alphabet_map) {
    oss << "  main_non_terminal = "
        << alphabet_map.at(main_non_terminal) << "\n";
    vector<pair<string, string>> ba_pair_strings;
    for (auto& item : branching_alphabets) {
      ba_pair_strings.push_back(make_pair(
          helpers::GetAlphabetString(alphabet_map.at(item.first)),
          helpers::GetAlphabetString(alphabet_map.at(item.second))));
    }
    oss << "  branching_alphabets = " << ba_pair_strings << "\n";
  } else {
    oss << "  main_non_terminal = " << main_non_terminal << "\n";
    oss << "  branching_alphabets = " << branching_alphabets << "\n";
  }
  oss << "  rules = {\n";
  for (auto& item : rules) {
    if (has_alphabet_map) {
      oss << "    " << alphabet_map.at(item.first) << " ::= ";
      oss << item.second.DebugString(alphabet_map) << "\n";
    } else {
      oss << "    " << item.first << " ::= ";
      oss << item.second.DebugString() << "\n";
    }
  }
  oss << "  }\n";
  auto a_map = alphabet_map;
  std::for_each(a_map.begin(), a_map.end(),
                 [](pair<const int, string>& p) {
                   p.second = helpers::GetAlphabetString(p.second);
                 });
  oss << "  alphabet_map = " << a_map << "\n";
  oss << "}\n";
  return oss.str();
}

std::size_t AParseGrammar::GetHash() const {
  return qk::HashFunction(make_tuple(alphabet_size,
                                     main_non_terminal,
                                     qk::HashFunction(rules),
                                     qk::HashFunction(branching_alphabets)));
}

}  // namespace aparse
