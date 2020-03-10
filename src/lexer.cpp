// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include "aparse/lexer.hpp"

#include <memory>

namespace aparse {

bool Lexer::Finalize() {
  APARSE_ASSERT(machine.initialized);
  APARSE_ASSERT(section_to_start_state_mapping.size() > 0);
  APARSE_ASSERT(qk::ContainsKey(section_to_start_state_mapping, main_section));
  APARSE_ASSERT(pattern_actions.size() > 0);
  initialized = true;
  return true;
}

}  // namespace aparse
