#include <iostream>

using namespace std;

struct S {int x;};

namespace A {
void G() {
  S* s = nullptr;
  cout << s->x << endl;
}
}

void F() {
  A::G();
}
