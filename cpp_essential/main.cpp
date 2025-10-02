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
void test_lambda_function();
void test_auto_return_type_deduct();

int main() {
  // test_cin();
  // test_init();
  // test_type_cast();
  // test_return_semantics();
  // test_function_pointer();
  // test_lambda_function();
  test_auto_return_type_deduct();
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

// -----------
struct Counter {
  int count = 0;

  void run() {
    auto inc = [this]() { count++; };
    inc();
    cout << count << endl;
  }
};

void test_lambda_function() {
  cout << "=== basic ===\n";
  auto say_hello = []() { cout << "Hello from a lambda!" << endl; };
  say_hello();
  auto add = [](int a, int b) -> int { return a + b; };
  auto sub = [](int a, int b) { return a - b; };
  cout << "3+4=" << add(3, 4) << endl << "3-4=" << sub(3, 4) << endl;

  cout << "=== capture list ===\n";
  int x = 10, y = 20;
  auto f1 = [x, y]() { return x + y; };
  auto f2 = [&x, &y]() { return ++x + ++y; };
  cout << f1() << ", " << f2() << endl;

  cout << "=== default capture ===\n";
  x = 1;
  y = 2;
  auto by_value = [=]() { return x + y; };
  auto by_ref = [&]() { return ++x + ++y; };
  auto mixed = [=, &y]() { return x + (++y); };
  cout << "(by value)x+y=" << by_value() << endl
       << "(by ref)++x + ++y=" << by_ref() << endl
       << "x + ++y=" << mixed() << endl;

  cout << "=== mutable lambda ===\n";
  int n = 5;
  auto f = [n]() mutable {
    n += 10; // lambdas that capture by value can NOT modify the copies unless
             // using mutable
    return n;
  };
  cout << "n=" << n << endl << "n+10=" << f() << endl;

  cout << "=== lambda as function arguments ===\n";
  vector<int> v = {1, 2, 3, 4, 5};
  sort(v.begin(), v.end(), [](int a, int b) { return a > b; });
  for (int x : v)
    cout << x << " ";

  cout << "\n=== lambda in std::function ===\n";
  function<int(int, int)> func = [](int a, int b) { return a + b; };
  cout << "func(10, 20)=" << func(10, 20) << endl;

  cout << "=== generic lambdas (cpp14) ===\n";
  auto add2 = [](auto a, auto b) { return a + b; };
  cout << "1+2=" << add2(1, 2) << endl << "1.5+2.5=" << add2(1.5, 2.5) << endl;

  cout << "=== capture this pointer ===\n";
  Counter c;
  c.run();
}

// -----------
auto mul(int a, double b) { return a * b; }
auto div2(int a, int b) -> double { return (double)a / b; }
auto foo(bool flag) {
  if (flag)
    return (double)1;
  else
    return 1.5;
}

int x = 10;
auto f() { return x; };             // return int
auto g() { return (x); };           // return int, drop reference
decltype(auto) h() { return (x); }; // return int&

template <typename T, typename U> auto add2(T a, U b) { return a + b; }

void test_auto_return_type_deduct() {
  cout << "=== single return ===\n";
  cout << "1*1.5=" << mul(1, 1.5) << endl << "3/2=" << div2(3, 2) << endl;

  cout << "=== multiple returns ===\n";
  cout << foo(true) << " " << foo(false) << endl;

  cout << "=== reference preservation ===\n";
  cout << "f()=" << f() << endl;
  cout << "g()=" << g() << endl;
  h() = 20;
  cout << "h()=" << h() << endl;

  cout << "=== template ===\n";
  cout << "1+2=" << add2(1, 2) << endl
       << "1.5+2.5=" << add2(1.5, 2.5) << endl
       << "1+1.5=" << add2(1, 1.5) << endl;
}
