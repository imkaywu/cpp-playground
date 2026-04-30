#include "concurrency_and_parallelism.cpp"
#include "core.cpp"
#include "design_pattern.cpp"
#include "memory_management.cpp"
#include "modern.cpp"
#include "oop.cpp"
#include "stl.cpp"
#include "template_generic_programming.cpp"

int main() {
#if defined(TEST_CR)
  CR::run();
#elif defined(TEST_OOP)
  OOP::run();
#elif defined(TEST_MM)
  MM::run();
#elif defined(TEST_TP)
  TP::run();
#elif defined(TEST_STL)
  STL::run();
#elif defined(TEST_MD)
  MD::run();
#elif defined(TEST_MT)
  MT::run();
#elif defined(TEST_DP)
  DP::run_visitor();
#endif

  return 0;
}
