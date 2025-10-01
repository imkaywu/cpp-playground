#include <iostream>
#include <string>
#include <utility>

using namespace std;

constexpr double PI = 3.14; // compile-time constant

void test_cin();
void test_init();
void test_type_cast();
void test_return_semantics();
void test_function_pointer();

int main() {
  // test_cin();
  // test_init();
  // test_type_cast();
  // test_return_semantics();
  test_function_pointer();
  return 0;
}

// -----------
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

// -----------
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

// -----------
class Base {
public:
  virtual void speak() { std::cout << "Base speaking\n"; }
  virtual ~Base() = default;
};

class Derived : public Base {
public:
  void speak() override { std::cout << "Derived speaking\n"; }
};

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

// -----------
struct Tracker {
  string name;

  Tracker(string n) : name(n) { cout << "Construct: " << name << endl; }
  Tracker(const Tracker &other) : name(other.name) {
    cout << "Copy: " << name << endl;
  }
  Tracker(Tracker &&other) noexcept : name(std::move(other.name)) {
    cout << "Move (noexcept): " << name << endl;
  }
  ~Tracker() { cout << "Destruct: " << name << endl; }
};

Tracker global_var("Global");

Tracker ReturnByValue() {
  Tracker t("Value");
  return t; // Return-Value-Optimization(RVO)/move applies
}

Tracker &ReturnByRef() { return global_var; }

const Tracker &ReturnByConstRef() { return global_var; }

Tracker *ReturnByPointer() {
  return new Tracker("Pointer"); // caller must delete
}

void test_return_semantics() {
  cout << "=== Return by Value ===\n";
  Tracker v = ReturnByValue();

  cout << "=== Return by Reference ===\n";
  Tracker &r = ReturnByRef();
  cout << "Got reference to: " << r.name << endl;

  cout << "=== Return by Const Reference ===\n";
  const Tracker &cr = ReturnByConstRef();
  cout << "Got const reference to: " << r.name << endl;

  cout << "=== Return by Pointer ===\n";
  Tracker *p = ReturnByPointer();
  cout << "Got pointer to: " << p->name << endl;
  delete p;
}

// -----------
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a, int b) { return a * b; }
int operate(int x, int y, int (*op)(int, int)) { return op(x, y); }

using FuncType = int (*)(int, int);

void test_function_pointer() {
  // retType (*name)(argTypes...)
  cout << "=== Basics ===\n";
  int (*func_ptr)(int, int);
  func_ptr = &add;
  cout << "2+3=" << func_ptr(2, 3) << endl
       << "4+7=" << (*func_ptr)(4, 7) << endl;

  // using/typedef
  cout << "=== using/typedef ===\n";
  FuncType ops[] = {add, sub, mul};
  cout << "1+2=" << ops[0](1, 2) << endl
       << "1-2=" << ops[1](1, 2) << endl
       << "1*2=" << ops[2](1, 2) << endl;

  // Pass function pointer
  cout << "=== pass function operator ===\n";
  cout << "3+4=" << operate(3, 4, add) << endl;

  // function/lambda
  cout << "=== function ===\n";
  function<int(int, int)> f = [](int a, int b) { return a + b; };
  cout << "2+3=" << f(2, 3) << endl;
}
