#include <cstdio>
#include <functional>
#include <iostream>
#include <string>

namespace CR {

// -----------
// cin
// -----------
void test_cin() {
  int a;
  std::string b;
  std::cin >> a >> b;
  std::cout << "a: " << a << ", b: " << b << "\n";

  std::string c;
  getline(std::cin, c);
  std::cout << "c: " << c << "\n";
}

// -----------
// type cast
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
  std::cout << "--- static_cast ---\n";
  double d = 3.14;
  int i = static_cast<int>(d);  // truncates to 3
  std::cout << "double: " << d << " -> int " << i << "\n";

  Derived der;
  Base *b = static_cast<Base *>(&der);
  b->speak();

  // safe cast within inheritance hierarchies.
  std::cout << "--- dynamic_cast ---\n";
  Base *poly = new Derived;
  Derived *down1 = dynamic_cast<Derived *>(poly);
  if (down1) {
    down1->speak();
  }

  Base *poly2 = new Base;
  Derived *down2 = dynamic_cast<Derived *>(poly2);
  if (!down2) {
    std::cout << "dynamic_cast failed, return nullptr\n";
  }
  delete poly;
  delete poly2;

  std::cout << "--- reinterpret_cast ---\n";
  int x = 65;
  char *px = reinterpret_cast<char *>(&x);
  std::cout << "int x = " << x << " first byte as char = " << *px << "\n";

  uintptr_t int_ptr = reinterpret_cast<uintptr_t>(&x);
  std::cout << "pointer as integer = " << int_ptr << "\n";

  std::cout << "--- const_cast ---\n";
  const char *msg = "hello";
  char *non_const = const_cast<char *>(msg);
  std::cout << "Original const char*: " << msg << "\n"
            << "After const_cast: " << non_const << "\n";
}

// -----------
// return semantics
// -----------
struct Tracker {
  std::string name;

  Tracker(std::string n) : name(n) {
    std::cout << "Construct: " << name << "\n";
  }
  Tracker(const Tracker &other) : name(other.name) {
    std::cout << "Copy: " << name << "\n";
  }
  Tracker(Tracker &&other) noexcept : name(std::move(other.name)) {
    std::cout << "Move (noexcept): " << name << "\n";
  }
  ~Tracker() { std::cout << "Destruct: " << name << "\n"; }
};

Tracker global_var("Global");

Tracker ReturnByValue() {
  Tracker t("Value");
  return t;  // Return-Value-Optimization(RVO)/move applies
}

Tracker &ReturnByRef() { return global_var; }

const Tracker &ReturnByConstRef() { return global_var; }

Tracker *ReturnByPointer() {
  return new Tracker("Pointer");  // caller must delete
}

void test_return_semantics() {
  std::cout << "--- Return by Value ---\n";
  Tracker v = ReturnByValue();

  std::cout << "--- Return by Reference ---\n";
  Tracker &r = ReturnByRef();
  std::cout << "Got reference to: " << r.name << "\n";

  std::cout << "--- Return by Const Reference ---\n";
  const Tracker &cr = ReturnByConstRef();
  std::cout << "Got const reference to: " << r.name << "\n";

  std::cout << "--- Return by Pointer ---\n";
  Tracker *p = ReturnByPointer();
  std::cout << "Got pointer to: " << p->name << "\n";
  delete p;
}

// -----------
// function pointer
// -----------
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a, int b) { return a * b; }
int operate(int x, int y, int (*op)(int, int)) { return op(x, y); }

using FuncType = int (*)(int, int);

void test_function_pointer() {
  // retType (*name)(argTypes...)
  std::cout << "--- Basics ---\n";
  int (*func_ptr)(int, int);
  func_ptr = &add;
  std::cout << "2+3=" << func_ptr(2, 3) << "\n"
            << "4+7=" << (*func_ptr)(4, 7) << "\n";

  // using/typedef
  std::cout << "--- using/typedef ---\n";
  FuncType ops[] = {add, sub, mul};
  std::cout << "1+2=" << ops[0](1, 2) << "\n"
            << "1-2=" << ops[1](1, 2) << "\n"
            << "1*2=" << ops[2](1, 2) << "\n";

  // Pass function pointer
  std::cout << "--- pass function operator ---\n";
  std::cout << "3+4=" << operate(3, 4, add) << "\n";

  // function/lambda
  std::cout << "--- function ---\n";
  std::function<int(int, int)> f = [](int a, int b) { return a + b; };
  std::cout << "2+3=" << f(2, 3) << "\n";
}

// -----------
// lambda function
// -----------
struct Counter {
  int count = 0;

  void run() {
    auto inc = [this]() { count++; };
    inc();
    std::cout << count << "\n";
  }
};

void test_lambda_function() {
  std::cout << "--- basic ---\n";
  auto say_hello = []() { std::cout << "Hello from a lambda!" << "\n"; };
  say_hello();
  auto add = [](int a, int b) -> int { return a + b; };
  auto sub = [](int a, int b) { return a - b; };
  std::cout << "3+4=" << add(3, 4) << "\n"
            << "3-4=" << sub(3, 4) << "\n";

  std::cout << "--- capture list ---\n";
  int x = 10, y = 20;
  auto f1 = [x, y]() { return x + y; };
  auto f2 = [&x, &y]() { return ++x + ++y; };
  std::cout << f1() << ", " << f2() << "\n";

  std::cout << "--- default capture ---\n";
  x = 1;
  y = 2;
  auto by_value = [=]() { return x + y; };
  auto by_ref = [&]() { return ++x + ++y; };
  auto mixed = [=, &y]() { return x + (++y); };
  std::cout << "(by value)x+y=" << by_value() << "\n"
            << "(by ref)++x + ++y=" << by_ref() << "\n"
            << "x + ++y=" << mixed() << "\n";

  std::cout << "--- mutable lambda ---\n";
  int n = 5;
  auto f = [n]() mutable {
    n += 10;  // lambdas that capture by value can NOT modify the copies unless
              // using mutable
    return n;
  };
  std::cout << "n=" << n << "\n"
            << "n+10=" << f() << "\n";

  std::cout << "--- lambda as function arguments ---\n";
  std::vector<int> v = {1, 2, 3, 4, 5};
  sort(v.begin(), v.end(), [](int a, int b) { return a > b; });
  for (int x : v) std::cout << x << " ";

  std::cout << "\n--- lambda in std::function ---\n";
  std::function<int(int, int)> func = [](int a, int b) { return a + b; };
  std::cout << "func(10, 20)=" << func(10, 20) << "\n";

  std::cout << "--- generic lambdas (cpp14) ---\n";
  auto add2 = [](auto a, auto b) { return a + b; };
  std::cout << "1+2=" << add2(1, 2) << "\n"
            << "1.5+2.5=" << add2(1.5, 2.5) << "\n";

  std::cout << "--- capture this pointer ---\n";
  Counter c;
  c.run();
}

// -----------
// auto return type deduction
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
auto f() { return x; };              // return int
auto g() { return (x); };            // return int, drop reference
decltype(auto) h() { return (x); };  // return int&

template <typename T, typename U>
auto add2(T a, U b) {
  return a + b;
}

void test_auto_return_type_deduct() {
  std::cout << "--- single return ---\n";
  std::cout << "1*1.5=" << mul(1, 1.5) << "\n"
            << "3/2=" << div2(3, 2) << "\n";

  std::cout << "--- multiple returns ---\n";
  std::cout << foo(true) << " " << foo(false) << "\n";

  std::cout << "--- reference preservation ---\n";
  std::cout << "f()=" << f() << "\n";
  std::cout << "g()=" << g() << "\n";
  h() = 20;
  std::cout << "h()=" << h() << "\n";

  std::cout << "--- template ---\n";
  std::cout << "1+2=" << add2(1, 2) << "\n"
            << "1.5+2.5=" << add2(1.5, 2.5) << "\n"
            << "1+1.5=" << add2(1, 1.5) << "\n";
}

// -----------
// basic pointer
// -----------
void allocate(int *p) { p = new int(10); }

void allocate(int **p) {
  *p = new int(10);  // modifies the pointer
}

void test_basic_pointer() {
  std::cout << "--- pointer ---\n";
  int x = 5;
  int *px = &x;
  int **ppx = &px;  // pointer to pointer
  std::cout << "x=" << x << "\n"
            << "*px=" << *px << "\n"
            << "**ppx=" << **ppx << "\n";

  std::cout << "--- dereference ---\n";
  *px = 20;
  std::cout << "x after *px=20 -> " << x << "\n";

  std::cout << "--- pointer to pointer ---\n";
  allocate(px);
  std::cout << "*px=" << *px << "\n";

  allocate(ppx);
  std::cout << "**ppx=" << **ppx << "\n";

  std::cout << "--- dangling pointer ---\n";
  int *heap_ptr = new int(99);
  std::cout << "*heap_ptr=" << *heap_ptr << "\n";
  delete heap_ptr;
  heap_ptr = nullptr;

  // *heap_ptr = 10 // dangling pointer, undefined behavior
}

// -----------
// NULL v nullptr
// -----------
void foo(int n) { std::cout << "int overload\n"; }

void foo(int *p) { std::cout << "int* overload\n"; }

void test_nullptr_NULL_macro() {
  foo(0);
  // NULL definition
  // - C (classic): (void*) 0
  // - C++: 0
  // foo(NULL); // ambiguous - could be int or pointer
  foo(nullptr);
}

// -----------
// new/delete
// -----------
void test_new_delete() {
  std::cout << "--- arrays ---\n";
  int *arr = new int[5]{1, 2, 3, 4, 5};
  for (int i = 0; i < 5; ++i) {
    std::cout << arr[i] << " ";
  }
  std::cout << "\n";

  delete[] arr;

  std::cout << "--- objects ---\n";
  Tracker *tracker = new Tracker("test new delete");
  delete tracker;
}

// -----------
// const correctness
// -----------
// "right-to-left" rule
void test_const_correctness() {
  int a = 10;
  int b = 20;

  std::cout << "--- pointer to const data ---\n";
  const int *p1 = &a;
  std::cout << "*p1=" << *p1 << "\n";

  // *p1 = 30; error: cannot modify data
  p1 = &b;
  std::cout << "*p1=" << *p1 << "\n";

  std::cout << "--- const pointer to non-const data ---\n";
  int *const p2 = &a;
  *p2 = 40;
  std::cout << "*p2=" << *p2 << "\n";
  // p2 = &b; // error: cannot modify pointer

  std::cout << "--- const pointer to const data ---\n";
  const int *const p3 = &a;
  std::cout << "*p3=" << *p3 << "\n";
  // *p3 = 50; error: cannot modify data
  // p3 = &b; error: cannot modify pointer
}

int run() {
  std::cout << "=== cin ===\n";
  test_cin();

  std::cout << "=== type cast ===\n";
  test_type_cast();

  std::cout << "=== return semantics ===\n";
  test_return_semantics();

  std::cout << "=== function pointer ===\n";
  test_function_pointer();

  std::cout << "=== lambda function ===\n";
  test_lambda_function();

  std::cout << "=== auto return type deduct ===\n";
  test_auto_return_type_deduct();

  std::cout << "=== basic pointer ===\n";
  test_basic_pointer();

  std::cout << "=== nullptr/NULL ===\n";
  test_nullptr_NULL_macro();

  std::cout << "=== new/delete ===\n";
  test_new_delete();

  std::cout << "=== const correctness ===\n";
  test_const_correctness();

  return 0;
}

}  // namespace CR
