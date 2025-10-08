#include <cstdio>
#include <iostream>
#include <memory>
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
void test_basic_pointer();
void test_nullptr_NULL_macro();
void test_new_delete();
void test_const_correctness();
void test_RAII();
void test_smart_pointer();
void test_oop();

int main() {
  // test_cin();
  // test_init();
  // test_type_cast();
  // test_return_semantics();
  // test_function_pointer();
  // test_lambda_function();
  // test_auto_return_type_deduct();
  // test_basic_pointer();
  // test_nullptr_NULL_macro();
  // test_new_delete();
  // test_const_correctness();
  // test_RAII();
  // test_smart_pointer();
  test_oop();
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

// -----------
void allocate(int* p) {
  p = new int(10);
}

void allocate(int **p) {
  *p = new int(10); // modifies the pointer
}

void test_basic_pointer() {
  cout << "=== pointer ===\n";
  int x = 5;
  int *px = &x;
  int **ppx = &px; // pointer to pointer
  cout << "x=" << x << endl
       << "*px=" << *px << endl
       << "**ppx=" << **ppx << endl;

  cout << "=== dereference ===\n";
  *px = 20;
  cout << "x after *px=20 -> " << x << endl;

  cout << "=== pointer to pointer ===\n";
  allocate(px);
  cout << "*px=" << *px << endl;

  allocate(ppx);
  cout << "**ppx=" << **ppx << endl;

  cout << "=== dangling pointer ===\n";
  int *heap_ptr = new int(99);
  cout << "*heap_ptr=" << *heap_ptr << endl;
  delete heap_ptr;
  heap_ptr = nullptr;

  // *heap_ptr = 10 // dangling pointer, undefined behavior
}

// -----------
void foo(int n) {
  cout << "int overload\n";
}

void foo(int* p) {
  cout << "int* overload\n";
}

void test_nullptr_NULL_macro() {
  foo(0);
  // NULL definition
  // - C (classic): (void*) 0
  // - C++: 0
  // foo(NULL); // ambiguous - could be int or pointer
  foo(nullptr);
}

// -----------
void test_new_delete() {
  cout << "=== arrays ===\n";
  int *arr = new int[5]{1, 2, 3, 4, 5};
  for (int i = 0; i < 5; ++i) {
    cout << arr[i] << " ";
  }
  cout << endl;

  delete[] arr;

  cout << "=== objects ===\n";
  Tracker *tracker = new Tracker("test new delete");
  delete tracker;
}

// -----------
// "right-to-left" rule
void test_const_correctness() {
  int a = 10;
  int b = 20;

  cout << "=== pointer to const data ===\n";
  const int *p1 = &a;
  cout << "*p1=" << *p1 << endl;

  // *p1 = 30; error: cannot modify data
  p1 = &b;
  cout << "*p1=" << *p1 << endl;

  cout << "=== const pointer to non-const data ===\n";
  int* const p2 = &a;
  *p2 = 40;
  cout << "*p2=" << *p2 << endl;
  // p2 = &b; // error: cannot modify pointer

  cout << "=== const pointer to const data ===\n";
  const int* const p3 = &a;
  cout << "*p3=" << *p3 << endl;
  // *p3 = 50; error: cannot modify data
  // p3 = &b; error: cannot modify pointer
}

// -----------
class FileRAII {
  FILE *file;

public:
  FileRAII(const char* name, const char* mode) {
    file = fopen(name, mode);
    if (!file) throw runtime_error("Failed to open file");
    cout << "File opended\n";
  }

  ~FileRAII() {
    if (file) {
      fclose(file);
      cout << "File closed\n";
    }
  }

  void write(const char* msg) {
    fprintf(file, "%s\n", msg);
  }

  // disable copying (if resource is unique)
  FileRAII(const FileRAII&) = delete;
  FileRAII& operator=(const FileRAII&) = delete;
};

void test_RAII() {
  try {
    FileRAII file("out.txt", "w");
    file.write("Hello RAII");
    
    // exception safety: destructor always runs, even if an exception is thrown
    throw runtime_error("Simulated exception"); 
                                                
                                                
  } catch(...) {
    cout << "Exception caught\n";
  }
}

// -----------
struct Node {
  string name;
  shared_ptr<Node> next;

  Node(string n) : name(n) {
    cout << "Construct node: " << name << endl;
  }

  ~Node() {
    cout << "Destroy node: " << name << endl;
  }
};

void test_smart_pointer() {
  using std::make_shared;

  cout << "=== Create two nodes ===\n";
  auto a = make_shared<Node>("A"); // a.use_count == 1
  auto b = make_shared<Node>("B"); // b.use_count == 1
  
  weak_ptr<Node> wa = a;
  weak_ptr<Node> wb = b;

  cout << "Initial counts: a=" << a.use_count() << ", b=" << b.use_count() << endl;

  cout << "=== Link A -> B ===\n";
  a->next = b;
  cout << "After A -> B, a=" << a.use_count() << ", b=" << b.use_count() << endl;

  cout << "=== Link B -> A (create cycle) ===\n";
  b->next = a;
  cout << "After B -> A, a=" << a.use_count() << ", b=" << b.use_count() << endl;

  cout << "=== Reset local shared_ptrs a & b ===\n";
  a.reset(); // a stops owning the object -> strong_count decrease by 1
  b.reset();
  if (!a) {
    cout << "After reset locals, a is nullptr, a.use_count=" << a.use_count() << endl;
  }
  cout << "After reset locals, wa.use_count=" << wa.use_count() << ", wb.use_count=" << wb.use_count() << endl;

  cout << "=== Break the cycle by resetting internal 'next' links ===\n";
  if (auto sa = wa.lock()) {
    sa->next.reset(); // release shared_ptr to B
  }
  if (auto sb = wb.lock()) {
    sb->next.reset(); /// release shared_ptr to A
  }
  cout << "After breaking internal links, wa.use_count=" << wa.use_count() << ", wb.use_count=" << wb.use_count() << endl;
}

// -----------
// Base class with polymorphism
class Shape {
public:
  Shape() { cout << "Construct Shape\n"; }
  virtual void draw() const = 0; // pure virtual -> abstract class
  virtual ~Shape() { cout << "Destruct Shape\n"; }
};

class Circle : public Shape {
  int radius;

public:
  Circle(int r) : radius(r) { cout << "Construct Circle\n"; }
  void draw() const override { cout << "Draw circle with radius: " << radius << endl; }
  ~Circle() { cout << "Destruct Circle\n"; }

  friend void showRadius(const Circle&);
};

void showRadius(const Circle& c) {
  cout << "Circle radius: " << c.radius << endl;
}

// Class with constructors/desctructors/copy/move
class Resource {
// Private does not mean “object-private.” It means “class-private.”
private:
  int *data;

public:
  static int count;

  Resource(int val = 0) {
    data = new int(val);
    count++;
    cout << "Construct Resource, value=" << *data << endl;
  }

  // copy constructor
  Resource(const Resource& other) {
    data = new int(*other.data);
    count++;
    cout << "Copy construct Resource, value=" << *data << endl;
  }

  // move constructor
  Resource(Resource&& other) noexcept {
    data = other.data;
    other.data = nullptr;
    count++;
    cout << "Move construct Resource, value=" << *data << endl;
  }

  // copy assignment
  Resource& operator=(const Resource& other) {
    if (this != &other) {
      delete data;
      data = new int(*other.data);
    }
    cout << "Copy assigned Resource, value=" << *data << endl;
    return *this;
  }

  // move assignment
  Resource& operator=(Resource&& other) noexcept{
    if (this != &other) {
      delete data;
      data = other.data;
      other.data = nullptr;
    }
    cout << "Move assigned Resource, value=" << *data << endl;
    return *this;
  }

  // Destructor
  ~Resource() {
    if (data) {
      cout << "Destruct resource, value=" << *data << endl;
      delete data;
    } else {
      cout << "Destruct resource, value nullptr" << endl;
    }
    count--;
  }

  int get() const { return *data; }

  Resource operator+(const Resource& other) const {
    return Resource(*data + *other.data);
  }

  static int get_count() { return count; }
};

class Manager {
  unique_ptr<Resource> res;

public:
  Manager(int val) : res(make_unique<Resource>(val)) {}
  void show() { cout << "Managed resource = " << res->get() << endl; }
};

int Resource::count = 0; // static members need a definition in a source file

void test_oop() {
  cout << "=== Constructors/Copy/Move ===\n";
  Resource r1(10);
  Resource r2(20);
  Resource r3 = r1; // copy construct
  Resource r4 = r1 + r2; // operator+
  r3 = r2; // copy assign
  Resource r5 = std::move(r1); // move construct
  r5 = std::move(r2); // move assign
  cout << "Current resource count: " << Resource::get_count() << endl;

  cout << "=== Polymorphism ===\n";
  unique_ptr<Shape> shape = make_unique<Circle>(5);
  shape->draw();
  Circle* circle = dynamic_cast<Circle*>(shape.get());
  showRadius(*circle);

  cout << "=== RAII & smart pointer ===\n";
  Manager mgr(99);
  mgr.show();

  cout << "=== Destruct ===\n";
}

