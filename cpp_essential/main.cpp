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
#include "modern.cpp"
#include "oop.cpp"
#include "singleton.cpp"
#include "template_generic_programming.cpp"

using namespace std;

constexpr double PI = 3.14;  // compile-time constant

void test_singleton();
void test_if_constexpr();

int main() {
  // test_singleton();
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
