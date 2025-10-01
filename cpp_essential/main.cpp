#include <iostream>

using namespace std;

constexpr double PI = 3.14; // compile-time constant

void test_cin();
void test_init();
void test_type_cast();

class Base {
public:
  virtual void speak() { std::cout << "Base speaking\n"; }
  virtual ~Base() = default;
};

class Derived : public Base {
public:
  void speak() override { std::cout << "Derived speaking\n"; }
};

int main() {
  // test_cin();
  // test_init();
  test_type_cast();
  return 0;
}

void test_cin() {
  cout << "=== cin ===\n";

  int a;
  string b;
  cin >> a >> b;
  cout << "a: " << a << ", b: " << b << endl;

  string c;
  getline(cin, c);
  cout << "c: " << c << endl;
}

void test_init() {
  cout << "=== initialization ===\n";

  // =
  {
    int x = 5;
    // int y = 3.14; // allow narrowing conversion, warning
    string a = "hello";
    cout << "=: " << x << " " << a << endl;
  }

  // (): calls a constructor
  {
    int x(5);
    // int y(3.14); // allow narrowing conversion, warning
    string a("hello");
    cout << "(): " << x << " " << a << endl;
  }

  // {}: cpp11
  {
    int x{5};
    // int y{3.14}; // ERROR: narrowing double -> int
    string a{"hello"};
    cout << "{}: " << x << " " << a << endl;
  }
}

void test_type_cast() {
  cout << "=== static_cast ===\n";
  double d = 3.14;
  int i = static_cast<int>(d); // truncates to 3
  cout << "double: " << d << " -> int " << i << endl;

  Derived der;
  Base *b = static_cast<Base *>(&der);
  b->speak();

  // safe cast within inheritance hierarchies.
  cout << "=== dynamic_cast ===\n";
  Base *poly = new Derived;
  Derived *down1 = dynamic_cast<Derived *>(poly);
  if (down1) {
    down1->speak();
  }

  Base *poly2 = new Base;
  Derived *down2 = dynamic_cast<Derived *>(poly2);
  if (!down2) {
    cout << "dynamic_cast failed, return nullptr\n";
  }
  delete poly;
  delete poly2;

  cout << "=== reinterpret_cast ===\n";
  int x = 65;
  char *px = reinterpret_cast<char *>(&x);
  cout << "int x = " << x << " first byte as char = " << *px << endl;

  uintptr_t int_ptr = reinterpret_cast<uintptr_t>(&x);
  cout << "pointer as integer = " << int_ptr << endl;

  cout << "=== const_cast ===\n";
  const char *msg = "hello";
  char *non_const = const_cast<char *>(msg);
  cout << "Original const char*: " << msg << endl
       << "After const_cast: " << non_const << endl;
}
