#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace TP {

// ------------
// Function templates
// ------------
template <typename T, typename U>
T clamp(T value, U low, U high) {
  using Common = decltype(value + low);

  return value < low ? low : (value > high ? high : value);
}

// ------------
// Class templates
// ------------
template <typename T>
class SimpleVector {
 private:
  T* data;
  size_t size;
  size_t capacity;

 public:
  SimpleVector() : data(nullptr), size(0), capacity(0) {}

  ~SimpleVector() { delete[] data; }

  void push_back(const T& value) {
    if (size == capacity) {
      capacity = capacity == 0 ? 1 : capacity * 2;
      T* new_data = new T[capacity];

      for (size_t i = 0; i < size; ++i) {
        new_data[i] = data[i];
      }

      delete[] data;
      data = new_data;
    }

    data[size++] = value;
  }

  T& operator[](size_t index) {
    if (index >= size) throw std::out_of_range("Index out of range");

    return data[index];
  }

  size_t get_size() const { return size; }
};

// ------------
// Template specialization
// ------------
template <typename T>
class Printer {
 public:
  static void print(const T& value) { std::cout << value << "\n"; }
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
struct IsPointer<T*> {
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
std::unique_ptr<T> make_unique_custom(Args&&... args) {
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
void inspect(T&& p) {
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

// ------------
// enable_if
// ------------
// c++ 11
//
// Conditionally enables functions/types at compile time.
// Give me type "T" ONLY if "T" is integral, otherwise function disappears
// (SFINAE)
// std::is_integral<T>::value -> true/false
// std::enable_if<cond, T>    -> gives type ONLY if cond == true
// ::type                     -> extract the type
// typename                   -> tell compiler it's a type
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
auto print_type2(const T& value) ->
    typename std::enable_if<std::is_integral<T>::value>::type {
  std::cout << "Integral type: " << value << "\n";
}

template <typename T>
auto print_type2(const T& value) ->
    typename std::enable_if<std::is_floating_point<T>::value>::type {
  std::cout << "Floating-point type: " << value << "\n";
}

template <typename T>
auto print_type2(const T& value) ->
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

// ------------
// run tests
// ------------
int run() {
  std::cout << "=== Function templates ===\n";
  std::cout << clamp(1, 0.0, 2.0) << "\n";

  std::cout << "=== Class templates ===\n";
  SimpleVector<int> v;
  v.push_back(10);
  v.push_back(20);
  std::cout << v[1] << "\n";

  SimpleVector<std::string> vs;
  vs.push_back("hello");
  std::cout << vs[0] << "\n";

  std::cout << "=== Template specialization ===\n";
  Printer<int>::print(10);
  Printer<bool>::print(true);

  std::cout << "=== Partial specialization ===\n";
  std::cout << IsPointer<int>::value << "\n";
  std::cout << IsPointer<int*>::value << "\n";

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

  std::cout << "=== enable_if ===\n";
  std::cout << "5*5=" << square(5) << "\n";
  // square(3.14); // ERROR

  std::cout << "=== SFINAE ===\n";
  test_sfinae();

  return 0;
}

};  // namespace TP
