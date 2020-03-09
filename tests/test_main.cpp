// Copyright: 2015 Mohit Saini
// Author: Mohit Saini (mohitsaini1196@gmail.com)

#include <iostream>
#include "gtest/gtest.h"

namespace aparse {
void CrashSignalHandler(int sig) {
  std::cout << "Aborting ..." << std::endl;
  exit(sig);
}
}  // namespace aparse


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  for (auto& a : {SIGSEGV, SIGILL, SIGFPE, SIGABRT, SIGBUS, SIGTERM}) {
    signal(a, aparse::CrashSignalHandler);
  }
  return RUN_ALL_TESTS();
}
