#include <concepts>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace TP {

// ------------
// Function templates
// ------------

template <typename T>
T add(T a, T b) {
  std::cout << "[Template] add(T,T) called\n";
  return a + b;
}

int add(int a, int b) {
  std::cout << "[Overload] add(T,T) called\n";
  return a + b;
}

// Template Specialization: specialize (customize) the template's behavior for
// particular template arguments.
template <>
std::string add<std::string>(std::string a, std::string b) {
  std::cout << "[Specialization] add<string>(string,string) called\n";
  return a + " & " + b;
}

template <typename T, typename U>
auto multiply(T a, U b) {
  std::cout << "[Template] multiply(T,U) called\n";
  return a * b;
}

template <typename T, typename U>
T clamp(T value, U low, U high) {
  using Common = decltype(value + low);

  return value < low ? low : (value > high ? high : value);
}

void test_function_template() {
  std::cout << "--- Function Template Basics ---\n";
  std::cout << add(3, 4) << "\n"
            << add(2.5, 4.1) << "\n"
            << add(std::string("Hi"), std::string("Bob")) << "\n";

  std::cout << "--- Explicit Template Arguments ---\n";
  std::cout << add<int>(10.5, 20.9) << "\n";

  std::cout << "--- Multiple Type Parameters ---\n";
  std::cout << multiply(3, 4.5) << "\n";
  std::cout << clamp(1, 0.0, 2.0) << "\n";
}

// ------------
// Class templates
// ------------
template <typename T>
class SimpleVector {
 private:
  T *data;
  size_t size;
  size_t capacity;

 public:
  SimpleVector() : data(nullptr), size(0), capacity(0) {}

  ~SimpleVector() { delete[] data; }

  void push_back(const T &value) {
    if (size == capacity) {
      capacity = capacity == 0 ? 1 : capacity * 2;
      T *new_data = new T[capacity];

      for (size_t i = 0; i < size; ++i) {
        new_data[i] = data[i];
      }

      delete[] data;
      data = new_data;
    }

    data[size++] = value;
  }

  T &operator[](size_t index) {
    if (index >= size) throw std::out_of_range("Index out of range");

    return data[index];
  }

  size_t get_size() const { return size; }
};

template <typename T>
class Box {
 protected:
  T value;

 public:
  Box(T v) : value(v){};
  T get() const { return value; }
  void set(T v) { value = v; }
};

template <typename T>
class SafeBox : public Box<T> {
  bool locked;

 public:
  SafeBox(T v) : Box<T>(v), locked(true) {}

  void lock() { locked = true; }
  void unlock() { locked = false; }

  T get_value() const {
    if (locked) {
      throw std::runtime_error("Box is locked");
    }
    return this->value;
  }
};

template <>
class SafeBox<std::string> : public Box<std::string> {
 public:
  SafeBox(std::string v) : Box<std::string>("Encrypted: " + v) {}
  std::string get_value() const { return this->value; }
};

void test_class_template() {
  SimpleVector<int> v;
  v.push_back(10);
  v.push_back(20);
  std::cout << v[1] << "\n";

  SimpleVector<std::string> vs;
  vs.push_back("hello");
  std::cout << vs[0] << "\n";

  SafeBox<int> safe_box_int(42);
  try {
    std::cout << safe_box_int.get_value() << "\n";
  } catch (const std::exception &e) {
    std::cout << e.what() << "\n";
  }

  safe_box_int.unlock();
  std::cout << safe_box_int.get_value() << "\n";

  SafeBox<std::string> safe_box_str("secret");
  std::cout << safe_box_str.get_value() << "\n";
}

// ------------
// Template specialization
// ------------
template <typename T>
class Printer {
 public:
  static void print(const T &value) { std::cout << value << "\n"; }
};

// Full specialization for bool
template <>
class Printer<bool> {
 public:
  static void print(bool value) {
    std::cout << (value ? "true" : "false") << "\n";
  }
};

// ------------
// Partial specialization
// ------------
template <typename T>
struct IsPointer {
  static constexpr bool value = false;
};

template <typename T>
struct IsPointer<T *> {
  static constexpr bool value = true;
};

template <typename T1, typename T2>
class Pair {
 public:
  void print() { std::cout << "General pair\n"; }
};

template <typename T>
class Pair<T, T> {
 public:
  void print() { std::cout << "Partial specialization: both types same\n"; }
};

// -----------
// Variadic templates
// https://eli.thegreenplace.net/2014/variadic-templates-in-c/
// -----------
void print() {
  std::cout << __PRETTY_FUNCTION__ << "\n";
  std::cout << "No more inputs\n";
}

// typename... Args: template parameter pack
template <typename T, typename... Args>
// Args... args: function parameter pack (expands into multiple parameters)
void print(T first, Args... args) {
  std::cout << __PRETTY_FUNCTION__ << "\n";
  std::cout << "First arg: " << first << "\n";
  // args...: expands into multiple arguments
  print(args...);
}

template <typename T>
bool pair_comparer(T a, T b) {
  return a == b;
}

template <typename T>
bool pair_comparer(T a) {
  return false;
}

template <typename T, typename... Args>
bool pair_comparer(T a, T b, Args... args) {
  return a == b && pair_comparer(args...);
}

template <typename... Args>
void print_fold(Args... args) {
  // comma operator: chain all those operations together inside a fold
  ((std::cout << args << " "), ...);
  std::cout << "\n";
}

template <typename... Args>
void count_args(Args... args) {
  std::cout << "Number of args: " << sizeof...(args) << "\n";
}

template <typename T, typename... Args>
std::unique_ptr<T> make_unique_custom(Args &&...args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class Person {
 public:
  Person(std::string n, int a) {
    std::cout << "Constructing " << n << ", " << a << "\n";
  }
};

void test_variadic_templates() {
  std::cout << "--- Expand a parameter pack ---\n";
  print(1, 2.5, "hello");

  std::cout << "pair comparer (even): " << pair_comparer(1.5, 1.5, 2, 2, 6, 6)
            << "\n";
  std::cout << "pair comparer (odd): " << pair_comparer(1.5, 1.5, 2, 2, 6, 6, 7)
            << "\n";

  std::cout << "--- Fold expression ---\n";
  print_fold(1, 2.5, "hello");

  std::cout << "--- Count args ---\n";
  count_args(1, 2.5, "hello");

  std::cout << "--- Perfect forwarding ---\n";
  make_unique_custom<Person>("Alice", 10);
}

// ------------
// Template type deduction
// ------------
// Rule 1: By value -> drops reference & const
//   template<typename T>
//   void f(T x);
//   const int a = 10;
//   f(a);  // T = int (const dropped)
// Rule 2: By reference -> preserves const
//   template<typename T>
//   void f(T& x);
//   const int a = 10;
//   f(a);  // T = const int
// Rule 3: universal refernce
//   template<typename T>
//   void f(T&& x);
//   int a = 10;
//   f(a);  // T = int&
//   f(1);  // T = int

template <typename T>
void inspect(T &&p) {
  if constexpr (std::is_lvalue_reference<T>::value) {
    std::cout << "T is lvalue reference\n";
  } else {
    std::cout << "T is NOT lvalue reference\n";
  }
}

// ------------
// decltype
// ------------
template <typename T, typename U>
auto add(T a, U b) -> decltype(a + b) {
  return a + b;
}

// -----------
// decltype/auto
// -----------
int global_x = 10;

int &get_ref() { return global_x; }
int get_val() { return global_x; }

template <typename Container>
decltype(auto) get_first(Container &&c) {
  return std::forward<Container>(c).front();
}

void test_decltype_auto() {
  {
    std::cout << "=== Basic ===\n";
    int x = 42;
    const double &y = 3.14;
    std::string s = "hello";

    decltype(x) a = 10;          // int
    decltype((x)) b = x;         // int&
    decltype(y) c = y;           // const double&
    decltype(s.size()) len = 5;  // size_t

    std::cout << a << " " << b << " " << c << " " << len << "\n";
  }

  {
    std::cout << "=== decltype(auto) ===\n";
    auto a = get_ref();
    decltype(auto) b = get_ref();

    a = 20;
    b = 30;

    std::cout << "global_x = " << global_x << "\n";
  }

  {
    std::vector<int> v = {1, 2, 3};
    std::cout << get_first(v) << "\n";

    get_first(v) = 42;
    std::cout << v[0] << "\n";

    const std::vector<int> cv = {4, 5, 6};
    std::cout << get_first(cv) << "\n";
  }
}

// ------------
// enable_if
// ------------
//
// Conditionally enables functions/types at compile time.
//
// Give me type "T" ONLY if "T" is integral, otherwise function disappears
//   std::is_integral<T>::value -> true/false
//   std::enable_if<cond, T>    -> gives type ONLY if cond == true
//   ::type                     -> extract the type
//   typename                   -> tell compiler it's a type
//
// Implementation:
//   template<bool, typename T>
//   struct enable_if {};
//
//   template<true, typename T>
//   struct enable_if<true, T> {
//     typedef T type;
//   };
//
// c++ 11
template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type square(T x) {
  return x * x;
}

// c++ 14
/*
template <typename T>
std::enable_if_t<std::is_integral<T>::value, T> square(T x) {
  return x * x;
}
*/

// ------------
// SFINAE
// ------------
//
// Substitution Failure Is Not An Error
// If template substitution fails -> compiler ignores that candidate
//
// Template substitution = the compiler replacing template parameters with
// actual types/values and checking if the resulting code is valid.
template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type increment(
    T value) {
  std::cout << "Integral verison called\n";
  return value + 1;
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type increment(
    T value) {
  std::cout << "Floating-point version called\n";
  return value + 0.5;
}

template <typename, typename = void>
struct has_size_method : std::false_type {};

template <typename T>
struct has_size_method<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type {};

template <typename T>
auto print_type2(const T &value) ->
    typename std::enable_if<std::is_integral<T>::value>::type {
  std::cout << "Integral type: " << value << "\n";
}

template <typename T>
auto print_type2(const T &value) ->
    typename std::enable_if<std::is_floating_point<T>::value>::type {
  std::cout << "Floating-point type: " << value << "\n";
}

template <typename T>
auto print_type2(const T &value) ->
    typename std::enable_if<!std::is_arithmetic<T>::value>::type {
  std::cout << "Other type: " << value << "\n";
}

void test_sfinae() {
  std::cout << "--- Basic ---\n";
  std::cout << increment(10) << "\n";
  std::cout << increment(10.0) << "\n";

  std::cout << "--- Detect if a class has a member function ---\n";
  std::cout << std::boolalpha;
  std::cout << has_size_method<int>::value << "\n";
  std::cout << has_size_method<std::vector<int>>::value << "\n";
  std::cout << has_size_method<std::string>::value << "\n";

  std::cout << "--- Function overloading with SFINAE ---\n";
  print_type2(42);
  print_type2(3.14);
  print_type2("Hello world");
}

// -----------
// requires clause
// -----------
template <typename T>
  requires std::integral<T>
T multiply_by_two(T x) {
  return x * 2;
}

template <std::integral T>
void print_type(T) {
  std::cout << "Integral type\n";
}

template <std::floating_point T>
void print_type(T) {
  std::cout << "Floating-point type\n";
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
T square2(T x) {
  return x * x;
}

auto add2(std::integral auto a, std::integral auto b) { return a + b; }

template <std::totally_ordered T>
class Range {
  T low, high;

 public:
  Range(T l, T h) : low(l), high(h) {}
  bool contains(const T &x) const { return x >= low && x <= high; }
};

void test_requires_clause() {
  std::cout << "--- Basic ---\n";
  std::cout << multiply_by_two(21) << "\n";
  // std::cout << multiply_by_two(3.14) << "\n"; // compile error (not integral)

  /*
  std::cout << "=== Define your own concept ===\n";
  std::cout << add2(3, 5) << "\n";
  std::cout << add2(3.1, 5.2) << "\n";
  */

  std::cout << "--- Use concept as function overload selector ---\n";
  print_type(42);
  print_type(4.2);

  std::cout << "--- Combining multiple constraints ---\n";
  std::cout << square2(5) << "\n";
  std::cout << square2(5.5) << "\n";

  std::cout << "--- Abbreviated function templates ---\n";
  std::cout << add2(10, 20) << "\n";
  // std::cout << add2(10.1, 20.2) << "\n"; // compile error

  std::cout << "--- Use concept in classes ---\n";
  Range<int> r(10, 20);
  std::cout << std::boolalpha;
  std::cout << r.contains(15) << "\n";
}

// -----------
// CRTP (Curiously Recurring Template Pattern)
// https://eli.thegreenplace.net/2011/05/17/the-curiously-recurring-template-pattern-in-c/
// -----------
//
// A class inherits from a template instantiated with itself.
//   - No virtual overhead
//   - Compile-time polymorphism
//
// Logging mixin
template <typename Derived>
class Logger {
 public:
  Logger() {
    std::cout << "Creating " << static_cast<Derived *>(this)->to_string()
              << "\n";
  }

  ~Logger() {
    std::cout << "Destroying " << static_cast<Derived *>(this)->to_string()
              << "\n";
  }
};

// Counter mixin
template <typename Derived>
class Counter {
 private:
  static int count;

 public:
  Counter() { ++count; }
  Counter(const Counter &) { ++count; }
  Counter(Counter &&) noexcept { ++count; }
  Counter &operator=(const Counter &other) { ++count; }
  ~Counter() { --count; }
  static int get_count() { return count; }
};

template <typename Derived>
int Counter<Derived>::count = 0;

// Addable mixin
template <typename Derived>
class Addable {
 public:
  Derived operator+(const Derived &other) const {
    Derived result = static_cast<const Derived &>(*this);
    result += other;
    return result;
  }
};

// Base CRTP class combining everything
template <typename Derived>
class Entity : public Logger<Derived>,
               public Counter<Derived>,
               public Addable<Derived> {
 public:
  void print() const {
    std::cout << static_cast<const Derived *>(this)->to_string() << "\n";
  }
};

// Derived class
class Money : public Entity<Money> {
 public:
  int value;

  Money(int v = 0) : value(v) {}

  // Required by Addable mixin
  Money operator+(const Money &other) {
    Money res = static_cast<const Money &>(*this);
    res.value += other.value;
    return res;
  }

  // Required by Logger mixin
  std::string to_string() const {
    return "Money (" + std::to_string(value) + ")";
  }
};

void test_crtp() {
  Money m1(30);
  Money m2(50);

  Money m3 = m1 + m2;  // call operator+ to create a temp Money instance, then
                       // copied to m3
  m3.print();

  std::cout << "Money instances: " << Money::get_count() << "\n";

  {
    Money m4(100);
    std::cout << "Money instances (inside block): " << Money::get_count()
              << "\n";
  }

  std::cout << "Money instances (outside block): " << Money::get_count()
            << "\n";
}

// ------------
// Template Metaprogramming Basics
// ------------
//
// Compute values/types at compile time

template <int N>
struct Factorial {
  static constexpr int value = N * Factorial<N - 1>::value;
};

template <>
struct Factorial<0> {
  static constexpr int value = 1;
};

// ------------
// Fold Expressions
// ------------
// Reduce a parameter pack into a single value, simplify variadic templates.

template <typename... Args>
auto sum(Args... args) {
  return (args + ...);
}

// ------------
// Type Traits
// ------------

template <typename T>
void check() {
  if constexpr (std::is_pointer_v<T>) {
    std::cout << "Pointer\n";
  } else {
    std::cout << "Not pointer\n";
  }
}

// -----------
// Non-Type Template Parameter (NTTP)
// -----------
template <size_t N>
void repeat_hello() {
  for (size_t i = 0; i < N; ++i) {
    std::cout << "Hello!\n";
  }
}

template <typename T, size_t N>
// arr: a reference to a const array of size N
void print_array(const T (&arr)[N]) {
  std::cout << "Array size = " << N << " => ";
  for (size_t i = 0; i < N; ++i) {
    std::cout << arr[i] << " ";
  }
  std::cout << "\n";
}

template <typename T, size_t N>
class FixedArray {
  T arr[N];

 public:
  void fill(const T &val) {
    for (size_t i = 0; i < N; ++i) {
      arr[i] = val;
    }
  }

  void show() const {
    std::cout << "Array size = " << N << " => ";
    for (size_t i = 0; i < N; ++i) {
      std::cout << arr[i] << " ";
    }
    std::cout << "\n";
  }
};

void test_nttp() {
  std::cout << "--- Basic ---\n";
  repeat_hello<4>();

  std::cout << "--- Array reference ---\n";
  int arr[4] = {1, 2, 3, 4};
  print_array<int, 4>(arr);

  std::cout << "--- Class template with non-type parameter ---\n";
  FixedArray<int, 4> fixed_arr;
  fixed_arr.fill(10);
  fixed_arr.show();
}

// -----------
// Concept
// -----------
// A concept that requires type T to support operator+
// C++20
/*
template <typename T>
concept Addable = std::requires(T a, T b) {
  { a + b } -> std::convertible_to<T>;
}

template <Addable T>
T add3(T a, T b) {
  return a + b;
}
*/

// ------------
// run tests
// ------------
int run() {
  std::cout << "=== Function templates ===\n";
  test_function_template();

  std::cout << "=== Class templates ===\n";
  test_class_template();

  std::cout << "=== Template specialization ===\n";
  Printer<int>::print(10);
  Printer<bool>::print(true);

  std::cout << "=== Partial specialization ===\n";
  std::cout << IsPointer<int>::value << "\n";
  std::cout << IsPointer<int *>::value << "\n";

  Pair<int, double> p1;
  Pair<int, int> p2;
  p1.print();
  p2.print();

  std::cout << "=== Variadict templates ===\n";
  test_variadic_templates();

  std::cout << "=== Template type deduction ===\n";
  int x = 10;
  inspect(x);   // lvalue -> int&
  inspect(20);  // rvalue -> int

  std::cout << "=== decltype ===\n";
  decltype(x) a = 10;   // int
  decltype((x)) b = x;  // int&
  b = 50;
  std::cout << "x: " << x << "\n";

  test_decltype_auto();

  std::cout << "=== enable_if ===\n";
  std::cout << "5*5=" << square(5) << "\n";
  // square(3.14); // ERROR

  std::cout << "=== SFINAE ===\n";
  test_sfinae();

  std::cout << "=== requires clause ===\n";
  test_requires_clause();

  std::cout << "=== CRTP (Curiously Recurring Template Pattern) ===\n";
  test_crtp();

  std::cout << "=== Template Metaprogramming Basic ===\n";
  std::cout << "5! = " << Factorial<5>::value << "\n";

  std::cout << "=== Fold Expressions ===\n";
  std::cout << "1+2+3+4=" << sum(1, 2, 3, 4) << "\n";

  std::cout << "=== Type Trait ===\n";
  check<int>();
  check<int *>();

  std::cout << "=== NTTP ===\n";
  test_nttp();

  std::cout << "=== Concept (TBD) ===\n";

  return 0;
}

};  // namespace TP
