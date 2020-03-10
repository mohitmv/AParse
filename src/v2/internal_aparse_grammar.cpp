// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "src/v2/internal_aparse_grammar.hpp"

#include <algorithm>
#include <functional>
#include <unordered_set>

#include <quick/unordered_map.hpp>
#include <quick/unordered_set.hpp>
#include <quick/stl_utils.hpp>
#include <quick/debug_stream.hpp>

#include "src/utils.hpp"

namespace aparse {
namespace v2 {

namespace {
int GetMaxNonTerminal(const AParseGrammar& grammar) {
  int output = 0;
  for (auto& rule : grammar.rules) {
    output = std::max(output, rule.first);
  }
  return output;
}

// Optimize the expression `CONCAT(x1, x2, x3, ..x'n)` for the case when n <= 1
void ConcatExprList(vector<Regex>* expr_list, Regex* output) {
  if (expr_list->size() == 0) {
    output->type = Regex::EPSILON;
  } else if (expr_list->size() == 1) {
    *output = std::move(expr_list->at(0));
  } else {
    output->type = Regex::CONCAT;
    output->children = std::move(*expr_list);
  }
}

void ComputeDependency(const Regex& regex,
                       const std::unordered_set<int>& non_terminals,
                       std::unordered_set<int>* output) {
  std::function<void(const Regex& regex)> lCompute = [&](const Regex& regex) {
    switch (regex.type) {
      case Regex::EPSILON:
        break;
      case Regex::ATOMIC: {
        if (qk::ContainsKey(non_terminals, regex.alphabet)) {
          output->insert(regex.alphabet);
        }
        break;
      }
      case Regex::UNION:
      case Regex::CONCAT:
      case Regex::KPLUS:
      case Regex::KSTAR: {
        for (auto& c : regex.children) {
          lCompute(c);
        }
        break;
      }
      default:
        assert(false);
    }
  };
  lCompute(regex);
}

}  // namespace

// @id_counter : enclosed_non_terminal_id_counter
void InternalAParseGrammar::ConstructEnclosedNonTerminals(
    std::vector<pair<int, Regex>>* rules_list,
    int* id_counter) {
  qk::unordered_map<std::pair<Alphabet, Regex>, int> unique_ent_map;
  auto lAddNewEnclosedNonTerminal = [&](const Regex& regex, Alphabet a) {
    auto p = make_pair(a, regex);
    if (not qk::ContainsKey(unique_ent_map, p)) {
      auto e_non_terminal = (*id_counter)++;
      unique_ent_map[p] = e_non_terminal;
      enclosed_rules[e_non_terminal] = regex;
      enclosed_ba_alphabet[e_non_terminal] = a;
    }
    return unique_ent_map.at(p);
  };
  std::function<void(Regex* regex)> lConstruct = [&](Regex* regex) {
    switch (regex->type) {
      case Regex::ATOMIC:
      case Regex::EPSILON: break;
      case Regex::UNION:
      case Regex::KPLUS:
      case Regex::KSTAR:
      case Regex::CONCAT: {
        for (auto& child : regex->children) {
          lConstruct(&child);
        }
        if (regex->type == Regex::CONCAT) {
          auto& children = regex->children;
          vector<vector<Regex>> stack;
          vector<int> b_alphabets;
          stack.resize(1);
          for (int i = 0; i < children.size(); i++) {
            auto& child = children[i];
            if (IsBAStart(child)) {
              b_alphabets.push_back(child.alphabet);
              stack.resize(stack.size() + 1);
            } else if (b_alphabets.size() > 0 and
                       IsBAEnd(child, b_alphabets.back())) {
              auto expr_list = std::move(stack.back());
              stack.pop_back();
              Regex s_regex;
              ConcatExprList(&expr_list, &s_regex);
              int ent = lAddNewEnclosedNonTerminal(s_regex, b_alphabets.back());
              stack.back().emplace_back(Regex(ent));
              b_alphabets.pop_back();
            } else {
              stack.back().emplace_back(std::move(child));
            }
          }
          ConcatExprList(&stack.back(), regex);
        }
        break;
      }
      default: assert(false);
    }
  };
  for (auto& item : *rules_list) {
    lConstruct(&item.second);
  }
}


bool InternalAParseGrammar::IsBAStart(const Regex& r) const {
  return (r.type == Regex::ATOMIC && qk::ContainsKey(ba_map, r.alphabet));
}

bool InternalAParseGrammar::IsBAEnd(const Regex& r) const {
  return (r.type == Regex::ATOMIC &&
          qk::ContainsKey(ba_inverse_map, r.alphabet));
}

bool InternalAParseGrammar::IsBAEnd(const Regex& r, Alphabet start_ba) const {
  return (r.type == Regex::ATOMIC &&
          qk::ContainsKey(ba_inverse_map, r.alphabet) &&
          ba_map.at(start_ba) == r.alphabet);
}

void InternalAParseGrammar::Init(const AParseGrammar& grammar) {
  grammar.Validate();
  // Step-1: Initialize the branching-alphabets-map.
  for (auto& item : grammar.branching_alphabets) {
    ba_map.insert(item);
    ba_inverse_map[item.second] = item.first;
  }

  // Step-2: Construct the enclosed-non-terminals by extraing out the
  //         sub-expressions wrapped in branching alphabets.
  // @enclosed_nt_id_counter is a incremental-id counter for
  // enclosed-non-terminal
  int enclosed_nt_id_counter = GetMaxNonTerminal(grammar) + 1;
  auto rules_list = grammar.rules;
  ConstructEnclosedNonTerminals(&rules_list, &enclosed_nt_id_counter);

  // Step-3: Assign labels to preserve the rule-number in regex.
  int label_counter = 1;
  auto& label_map = regex_label_to_original_rule_number_mapping;
  unordered_map<int, vector<int>> target_expr;
  for (int i = 0; i < rules_list.size(); i++) {
    auto& rule = rules_list[i];
    auto& regex = rule.second;
    regex.label = label_counter++;
    label_map[regex.label] = i;
    target_expr[rule.first].push_back(i);
  }
  for (auto& item : target_expr) {
    auto& expr = item.second;
    if (expr.size() == 1) {
      rules[item.first] = std::move(rules_list[expr.at(0)].second);
    } else {
      auto& regex = rules[item.first];
      regex.type = Regex::CONCAT;
      for (auto e : expr) {
        regex.children.emplace_back(std::move(rules_list[e].second));
      }
    }
  }
  // ToDo(Mohit): Assign a label to enclosed_rules as well.

  // Step-4: Populate 'non_terminals' and 'enclosed_non_terminals'.
  qk::STLGetKeys(rules, &non_terminals);
  qk::STLGetKeys(enclosed_rules, &enclosed_non_terminals);

  // Step-5: Compute the dependency_graph and topologically sort the
  //         non-terminals.
  for (auto& rule : rules) {
    ComputeDependency(rule.second,
                      non_terminals,
                      &dependency_graph[rule.first]);
  }
  vector<int> cycle_path;
  bool status = utils::TopologicalSortingInGraph(
                    dependency_graph,
                    &topological_sorted_non_terminals,
                    &cycle_path);
  (void)status;
  APARSE_DEBUG_ASSERT(status);
  APARSE_DEBUG_ASSERT(topological_sorted_non_terminals.size() > 0);

  // Step-6: Misc steps
  main_non_terminal = grammar.main_non_terminal;
  alphabet_size = grammar.alphabet_size;
}

void InternalAParseGrammar::DebugStream(qk::DebugStream& ds) const {
  ds << "alphabet_size = " << alphabet_size << "\n"
     << "main_non_terminal = " << main_non_terminal << "\n"
     << "rules = " << rules << "\n"
     << "enclosed_rules = " << enclosed_rules << "\n"
     << "enclosed_ba_alphabet = " << enclosed_ba_alphabet << "\n"
     << "regex_label_map" << regex_label_to_original_rule_number_mapping << "\n"
     << "ba_map = " << ba_map << "\n"
     << "ba_inverse_map = " << ba_inverse_map << "\n"
     << "dependency_graph = " << dependency_graph << "\n"
     << "topological_sorted_non_terminals = "
          << topological_sorted_non_terminals << "\n"
     << "non_terminals = " << non_terminals << "\n"
     << "enclosed_non_terminals = " << enclosed_non_terminals;
}

}  // namespace v2
}  // namespace aparse
