#include <iostream>
using namespace std;

namespace A {

template<typename T>
T F(T x) {
  return x*3;
}

}


namespace B {

template<typename T>
T F(T x) {
  return x*2;
}

}


namespace A {

int G() {
  return F(10);
}

}



int main() {

  cout << A::G() << endl;

  // Json m = Json::object {{"a", "111"}, {"b", 22}};

  // cout << m.dump() << endl;
  // cout << m["a"].dump() << endl;

  // string s = qk::ReadFile("/tmp/null.py");

  // cout << s << endl;
  // cout << qk::ReadFile("/tmp/null.py09876") << endl;

  // vector<int> v = {11, 222, 3333, 44444, 555555};

  // cout << v << endl;

  // cout << "Saini Mohit" << endl;
  return 0;
}

