#include <iostream>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

using namespace std;

struct S {int x;};

S* s = nullptr;


void printStacktrace()
{
    void *array[20];
    size_t size = backtrace(array, sizeof(array) / sizeof(array[0]));
    cout << array[0] << endl;
    cout << array[1] << endl;
    backtrace_symbols_fd(array, size, STDERR_FILENO);
}

void signalHandler(int sig) {
  cout << "signalHandler " << sig << endl;
  printStacktrace();
  cout << "Exiting.." << endl;
  exit(sig);
}


namespace A {
void G();
}

void F();

int main() {
  // if (s == nullptr) {
  //   cout << "aaa" << endl;
  //   *a = 10;
  // }
  for (auto& a : {SIGSEGV, SIGILL, SIGFPE, SIGABRT, SIGBUS, SIGTERM}) {
    cout << "Registring with signal " << a << endl;
    signal(a, signalHandler);
  }
  // actual_terminate = std::set_terminate(terminateHandler);


  // std::terminate();
  cout << "Here" << endl;
  // std::abort();
  // throw std::runtime_error("Runtime error");

  cout << "Registred" << endl;
  F();
  cout << "bb" << endl;
  return 0;
}
