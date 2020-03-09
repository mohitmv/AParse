#include <iostream>
#include "aparse/utils/assert.hpp"

#include "gtest/gtest.h"

using namespace std;

int* a = nullptr;

TEST(A, B) {
  cout << "aa" << endl;
  // *a = 10;
  // APARSE_ASSERT(1 == 2);
  cout << "bb" << endl;
}
