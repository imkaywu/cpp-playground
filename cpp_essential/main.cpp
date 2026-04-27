#include <concepts>  // for C++20 concepts like std::integral
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "concurrency_and_parallelism.cpp"
#include "core.cpp"
#include "memory_management.cpp"
#include "modern_feat.cpp"
#include "oop.cpp"
#include "singleton.cpp"
#include "template_generic_programming.cpp"

using namespace std;

constexpr double PI = 3.14;  // compile-time constant

void test_singleton();
void test_nttp();
void test_decltype_auto();
void test_if_constexpr();

int main() {
  // test_singleton();
  // test_nttp();
  // test_decltype_auto();
  // test_if_constexpr();

#if defined(TEST_CR)
  CR::run();
#elif defined(TEST_OOP)
  OOP::run();
#elif defined(TEST_MM)
  MM::run();
#elif defined(TEST_TP)
  TP::run();
#elif defined(TEST_MD)
  MD::run();
#elif defined(TEST_MT)
  MT::run();
#endif

  return 0;
}

// -----------
// Singleton
// -----------
void test_singleton() {
  auto &logger = Logger::Instance();

  logger.Info("This is info");
  logger.Error("This is error");
}

// -----------
// dynamic dispatch (vtable)
// -----------
// dynamic dispatch: deciding function call at runtime
// vtable: Table of function pointers for virtual functions
// vptr: Hidden pointer in objects pointing to vtable

// -----------
// dynamic cast for RTTI
// -----------
// RTTI: Run-Time Type Info. Metadata for runtime type checking.
//       only available when base class has virtual function(s)
//       vptr → vtable → RTTI descriptor

// -----------
// Non-Type Template Parameter
// -----------
template <size_t N>
void repeat_hello() {
  for (size_t i = 0; i < N; ++i) {
    cout << "Hello!" << endl;
  }
}

template <typename T, size_t N>
// arr: a reference to a const array of size N
void print_array(const T (&arr)[N]) {
  cout << "Array size = " << N << " => ";
  for (size_t i = 0; i < N; ++i) {
    cout << arr[i] << " ";
  }
  cout << endl;
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
    cout << "Array size = " << N << " => ";
    for (size_t i = 0; i < N; ++i) {
      cout << arr[i] << " ";
    }
    cout << endl;
  }
};

void test_nttp() {
  cout << "=== Basic ===\n";
  repeat_hello<4>();

  cout << "=== Array reference ===\n";
  int arr[4] = {1, 2, 3, 4};
  print_array<int, 4>(arr);

  cout << "=== Class template with non-type parameter ===\n";
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
T add_concept(T a, T b) {
  return a + b;
}
*/

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
    cout << "=== Basic ===\n";
    int x = 42;
    const double &y = 3.14;
    string s = "hello";

    decltype(x) a = 10;          // int
    decltype((x)) b = x;         // int&
    decltype(y) c = y;           // const double&
    decltype(s.size()) len = 5;  // size_t

    cout << a << " " << b << " " << c << " " << len << endl;
  }

  {
    cout << "=== decltype(auto) ===\n";
    auto a = get_ref();
    decltype(auto) b = get_ref();

    a = 20;
    b = 30;

    cout << "global_x = " << global_x << endl;
  }

  {
    vector<int> v = {1, 2, 3};
    cout << get_first(v) << endl;

    get_first(v) = 42;
    cout << v[0] << endl;

    const vector<int> cv = {4, 5, 6};
    cout << get_first(cv) << endl;
  }
}

// -----------
// if constexpr
// -----------
template <typename T>
void print_info(const T &value) {
  if constexpr (std::is_arithmetic_v<T>) {
    cout << "Arithmetic value: " << value << endl;
  } else if constexpr (std::is_same_v<T, string>) {
    cout << "String value: " << value << endl;
  } else {
    cout << "Other type" << endl;
  }
}

void test_if_constexpr() {
  print_info(42);
  print_info(3.14);
  print_info(string("Hi"));
  print_info(vector<int>{1, 2, 3});
}
