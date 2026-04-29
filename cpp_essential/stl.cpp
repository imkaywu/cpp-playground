#include <any>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <variant>

namespace STL {
/*
| Container     | random access | insert end     | insert middle | lookup   |
|---------------|---------------|----------------|---------------|----------|
| vector        | o(1)          | amortized o(1) | o(n)          | o(n)     |
| deque         | o(1)          | o(1)           | o(n)          | o(n)     |
| list          | o(n)          | o(1)           | o(1)*         | o(n)     |
| set           | o(log n)      | o(log n)       | o(log n)      | o(log n) |
| map           | o(log n)      | o(log n)       | o(log n)      | o(log n) |
| unordered_set | n/a           | avg o(1)       | avg o(1)      | avg o(1) |
| unordered_map | N/A           | Avg O(1)       | Avg O(1)      | Avg O(1) |
*/

// -----------
// lambdas
// -----------
//
// [capture](params) -> return_type { body }
//
// **no code**

// -----------
// Function object (functor)
// -----------
//
// class with operator()

// -----------
// std::function
// -----------
//
// Type-erased callable wrapper. Can store:
//   - function pointer
//   - lambda
//   - functor
//   - bind result

void test_function() {
  std::function<int(int, int)> op;

  op = [](int a, int b) { return a + b; };

  std::cout << op(2, 3) << "\n";
}

// -----------
// std::optional
// -----------
//
// Represent maybe value

std::optional<int> parse(bool ok) {
  if (ok) return 42;
  return std::nullopt;
}

void test_optional() {
  auto x = parse(true);

  if (x) std::cout << *x << "\n";
}

// -----------
// std::variant
// -----------
//
// Type-safe union

void test_variant() {
  std::variant<int, std::string> v;

  v = 42;
  v = "hello variant";

  std::visit([](auto&& x) { std::cout << x << "\n"; }, v);
}

// -----------
// std::any
// -----------
//
// Type-erased container.
// Stores any copyable type.
//

void test_any() {
  std::any a = 42;
  a = std::string("hello any");

  try {
    std::cout << std::any_cast<int>(a) << "\n";
  } catch (...) {
    std::cout << "bad cast\n";
  }

  try {
    std::cout << std::any_cast<std::string>(a) << "\n";
  } catch (...) {
    std::cout << "bad cast\n";
  }
}

int run() {
  std::cout << "=== std::function ===\n";
  test_function();

  std::cout << "=== std::optional ===\n";
  test_optional();

  std::cout << "=== std::variant ===\n";
  test_variant();

  std::cout << "=== std::any ===\n";
  test_any();

  return 1;
}
}  // namespace STL
