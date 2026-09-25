#include <concepts>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace TP {

// ============================================================
// FUNCTION TEMPLATES
// ============================================================
//
// A function template defines generic code for arbitrary types.
//
// The compiler instantiates the template for the concrete types
// used at the call site.
//
//     mymax(10, 20)       -> mymax<int>()
//     mymax(3.1, 4.2)     -> mymax<double>()
//
// The type can also be specified explicitly:
//
//     mymax<double>(10, 20.5)
// ============================================================

template <typename T>
T mymax(T a, T b) {
  return b < a ? a : b;
}

// Multiple template parameters allow arguments to have
// different types.
//
// The return type is deduced from the return expression.
// For example:
//
//     multiply(3, 4.5)
//         T = int
//         U = double
//         return type = double
//
template <typename T, typename U>
auto multiply(T a, U b) {
  return a * b;
}

// Ordinary overloads can coexist with function templates.
// A non-template overload can be preferred during overload
// resolution when it is a better match.
template <typename T>
T add(T a, T b) {
  std::cout << "[Template] add(T, T)\n";
  return a + b;
}

int add(int a, int b) {
  std::cout << "[Overload] add(int, int)\n";
  return a + b;
}

// Template Specialization: specialize (customize) the template's behavior for
// particular template arguments.
template <>
std::string add<std::string>(std::string a, std::string b) {
  std::cout << "[Specialization] add<string>(string,string) called\n";
  return a + " & " + b;
}

void example_function_templates() {
  std::cout << "\n=== Function Templates ===\n";

  std::cout << "--- Basics ---\n";

  std::cout << "mymax(int): " << mymax(3, 7) << '\n';

  std::cout << "mymax(double): " << mymax(3.14, 2.71) << '\n';

  std::string a = "hello";
  std::string b = "world";

  std::cout << "mymax(string): " << mymax(a, b) << '\n';

  // Explicit template argument.
  // Both arguments are converted to double before the
  // instantiated function is called.
  std::cout << "--- Explicit Template Arguments ---\n";

  std::cout << "mymax<double>: " << mymax<double>(10, 20.5) << '\n';

  std::cout << "--- Multiple Type Parameters ---\n";

  std::cout << "multiply(int, double): " << multiply(3, 4.5) << '\n';

  std::cout << "--- Overload & Specialization ---\n";

  // The non-template overload is preferred here.
  std::cout << "add(int, int): " << add(3, 4) << '\n';

  // The template is used for double.
  std::cout << "add(double, double): " << add(3.0, 4.0) << '\n';

  // The specialization is used for string.
  std::cout << "add(string, string): "
            << add(std::string("Hi"), std::string("Bob")) << "\n";
}

// ============================================================
// CLASS TEMPLATES
// ============================================================
//
// A class template generates a class for a particular type.
//
// Stack<T> is the canonical example:
//
//     Stack<int>
//     Stack<std::string>
//
// A class template does not necessarily require every operation
// supported by T. Requirements arise when the corresponding
// member function is instantiated/used.
// ============================================================

template <typename T>
class Stack {
 private:
  std::vector<T> elems;

 public:
  void push(const T& value) { elems.push_back(value); }

  T pop() {
    if (elems.empty()) {
      throw std::out_of_range("Stack is empty");
    }

    T value = elems.back();
    elems.pop_back();
    return value;
  }

  const T& top() const {
    if (elems.empty()) {
      throw std::out_of_range("Stack is empty");
    }

    return elems.back();
  }

  bool empty() const { return elems.empty(); }
};

// A second class template demonstrates a non-type parameter
// together with a type parameter.  NTTP is covered later in
// more detail.
template <typename T, std::size_t N>
class FixedArray {
 private:
  T data[N]{};

 public:
  void fill(const T& value) {
    for (std::size_t i = 0; i < N; ++i) {
      data[i] = value;
    }
  }

  constexpr std::size_t size() const { return N; }

  T& operator[](std::size_t index) { return data[index]; }

  const T& operator[](std::size_t index) const { return data[index]; }
};

void example_class_templates() {
  std::cout << "\n=== Class Templates ===\n";

  Stack<int> numbers;

  numbers.push(10);
  numbers.push(20);
  numbers.push(30);

  std::cout << "top = " << numbers.top() << '\n';

  std::cout << "pop = " << numbers.pop() << '\n';

  Stack<std::string> words;

  words.push("hello");
  words.push("world");

  std::cout << "top = " << words.top() << '\n';

  // Different template arguments create different types.
  FixedArray<int, 4> values;

  values.fill(42);

  std::cout << "FixedArray size = " << values.size() << '\n';

  std::cout << "values[2] = " << values[2] << '\n';
}

// ============================================================
// TEMPLATE SPECIALIZATION
// ============================================================
//
// A full specialization provides a completely different
// implementation for a particular template argument.
//
// Primary template:
//     Printer<T>
//
// Full specialization:
//     Printer<bool>
//
// This is different from overloading: specialization customizes
// an existing template for a specific template argument.
// ============================================================

template <typename T>
class Printer {
 public:
  static void print(const T& value) {
    std::cout << "generic: " << value << '\n';
  }
};

// Full specialization for bool.
template <>
class Printer<bool> {
 public:
  static void print(bool value) {
    std::cout << "bool: " << (value ? "true" : "false") << '\n';
  }
};

void example_template_specialization() {
  std::cout << "\n=== Template Specialization ===\n";

  Printer<int>::print(42);
  Printer<std::string>::print("hello");

  // Uses the specialized implementation.
  Printer<bool>::print(true);
}

// ============================================================
// PARTIAL SPECIALIZATION
// ============================================================
//
// Partial specialization is possible for class templates.
//
// The primary template:
//
//     IsPointer<T>
//
// handles arbitrary T.
//
// The partial specialization:
//
//     IsPointer<T*>
//
// matches every pointer type:
//
//     int*
//     double*
//     std::string*
//     ...
//
// Unlike full specialization, T is still a template parameter.
// ============================================================

template <typename T>
struct IsPointer {
  static constexpr bool value = false;
};

template <typename T>
struct IsPointer<T*> {
  static constexpr bool value = true;
};

// Another useful example:
//
// Pair<T1, T2>
//     generic implementation
//
// Pair<T, T>
//     partial specialization when both types are identical
//
template <typename T1, typename T2>
class Pair {
 public:
  void print() const { std::cout << "generic pair\n"; }
};

template <typename T>
class Pair<T, T> {
 public:
  void print() const { std::cout << "partial specialization: same types\n"; }
};

void example_partial_specialization() {
  std::cout << "\n=== Partial Specialization ===\n";

  std::cout << std::boolalpha;

  std::cout << "IsPointer<int>: " << IsPointer<int>::value << '\n';

  std::cout << "IsPointer<int*>: " << IsPointer<int*>::value << '\n';

  Pair<int, double> p1;
  Pair<int, int> p2;

  p1.print();
  p2.print();
}

// ============================================================
// VARIADIC TEMPLATES
// ============================================================
//
// A parameter pack represents zero or more parameters.
//
//     typename... Args
//
// declares a template parameter pack.
//
//     Args... args
//
// declares a function parameter pack.
//
//     args...
//
// expands the pack into individual arguments.
//
// Think "..." as:
//   - before identifier in template params → “pack of”
//   - after identifier/expression → “expand”
//
// https://eli.thegreenplace.net/2014/variadic-templates-in-c/
// ============================================================

void print() {
  // Base case: zero arguments.
  std::cout << __PRETTY_FUNCTION__ << "\n";
  std::cout << "No more inputs\n";
}

// |typename... Args|: **declares** that Args is a template type pack
template <typename T, typename... Args>
// |Args... args|: **declares** a function parameter pack named args (expands
// into multiple parameters)
void print(T first, Args... args) {
  std::cout << __PRETTY_FUNCTION__ << "\n";
  std::cout << "First arg: " << first << '\n';

  // ...: Expand args into separate function arguments.
  print(args...);
}

// sizeof...(pack) gives the number of elements in the pack.
//
// Normal sizeof: measures number of bytes.
//     sizeof(x)
//
// Pack sizeof: measures number of arguments.
//     sizeof...(args)
//
template <typename... Args>
void count_args(Args... args) {
  std::cout << "Number of args: " << sizeof...(args) << '\n';
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

// A practical example: construct an object by forwarding an
// arbitrary number of constructor arguments.
template <typename T, typename... Args>
std::unique_ptr<T> make_unique_custom(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class Person {
 public:
  Person(std::string name, int age) {
    std::cout << "Constructing " << name << ", age = " << age << '\n';
  }
};

void example_variadic_templates() {
  std::cout << "\n=== Variadic Templates ===\n";

  std::cout << "--- Expand a parameter pack ---\n";

  print(1, 2.5, "hello");

  std::cout << "pair comparer (even): " << pair_comparer(1.5, 1.5, 2, 2, 6, 6)
            << "\n";
  std::cout << "pair comparer (odd): " << pair_comparer(1.5, 1.5, 2, 2, 6, 6, 7)
            << "\n";

  std::cout << "--- Count args ---\n";

  count_args(1, 2.5, "hello");

  std::cout << "--- Perfect forwarding ---\n";

  auto person = make_unique_custom<Person>("Alice", 30);

  (void)person;
}

// ============================================================
// TEMPLATE TYPE DEDUCTION
// ============================================================
//
// The compiler deduces template parameters from function
// arguments.
//
// Important basic rules:
//
// 1. By value:
//      top-level const/reference is removed.
//
// 2. By reference:
//      const is preserved.
//
// 3. Forwarding reference T&&:
//      lvalue -> T = U&
//      rvalue -> T = U
//
// Reference collapsing:
//
//      &  + &  -> &
//      &  + && -> &
//      && + &  -> &
//      && + && -> &&
//
// The talk also demonstrates multiple template parameters and
// .
// ============================================================

template <typename T>
void by_value(T x) {
  std::cout << "by_value: " << __PRETTY_FUNCTION__ << '\n';

  (void)x;
}

template <typename T>
void by_reference(T& x) {
  std::cout << "by_reference: " << __PRETTY_FUNCTION__ << '\n';

  (void)x;
}

template <typename T>
void by_const_reference(const T& x) {
  std::cout << "by_const_reference: " << __PRETTY_FUNCTION__ << '\n';

  (void)x;
}

template <typename T>
void forwarding_reference(T&& x) {
  std::cout << "forwarding_reference: " << __PRETTY_FUNCTION__ << '\n';

  (void)x;
}

// Different template parameters can be deduced independently.
template <typename T1, typename T2>
void print_two(const T1& a, const T2& b) {
  std::cout << a << " / " << b << '\n';

  std::cout << "same type: " << std::boolalpha
            << std::is_same_v<T1, T2> << '\n';
}

void example_template_type_deduction() {
  std::cout << "\n=== Template Type Deduction ===\n";

  int a = 5;
  const int b = 10;

  std::cout << "--- By value ---\n";

  // T = int
  by_value(a);

  // T = int
  // top-level const is dropped.
  by_value(b);

  std::cout << "--- By reference ---\n";

  // T = int
  // parameter type = int&
  by_reference(a);

  // T = const int
  // parameter type = const int&
  by_reference(b);

  std::cout << "--- Const reference ---\n";

  // T = int
  // parameter type = const int&
  by_const_reference(a);

  // T = int
  // parameter type = const int&
  by_const_reference(b);

  std::cout << "--- Forwarding reference ---\n";

  // lvalue:
  // T = int&
  // parameter becomes int&
  forwarding_reference(a);

  // rvalue:
  // T = int
  // parameter becomes int&&
  forwarding_reference(10);

  std::cout << "--- Multiple template parameters ---\n";

  print_two(42, 3.14);
  print_two(10, 20);

  std::cout << "--- class template argument deduction (CTAD) ---\n";

  // Since C++17, the constructor can allow the compiler
  // to deduce the class template argument.
  std::vector values{10, 20, 30};

  std::cout << "vector size = " << values.size() << '\n';
}

// ============================================================
// decltype
// ============================================================
//
// decltype asks:
//
//     "What is the type of this expression?"
//
// Two particularly important cases:
//
//     decltype(x)
//         -> declared type of x
//
//     decltype((x))
//         -> expression category rules apply
//         -> often produces T& for an lvalue
//
// decltype(auto) lets a function preserve the exact return type
// of its return expression.
// ============================================================

int global_x = 10;

int& get_ref() { return global_x; }

int get_value() { return global_x; }

template <typename T, typename U>
auto add_decltype(T a, U b) -> decltype(a + b) {
  return a + b;
}

template <typename Container>
decltype(auto) get_first(Container&& container) {
  return std::forward<Container>(container).front();
}

void example_decltype() {
  std::cout << "\n=== decltype ===\n";

  std::cout << "--- Basic ---\n";

  int x = 42;
  const double& y = 3.14;
  std::string s = "hello";

  decltype(x) a = 10;          // int
  decltype((x)) b = x;         // int&
  decltype(y) c = y;           // const double&
  decltype(s.size()) len = 5;  // std::size_t

  std::cout << a << " " << b << " " << c << " " << len << '\n';

  std::cout << "--- decltype(auto) ---\n";

  // auto drops the reference returned by get_ref().
  auto value = get_ref();

  // decltype(auto) preserves the exact return type:
  // int&.
  decltype(auto) reference = get_ref();

  value = 20;
  reference = 30;

  std::cout << "global_x = " << global_x << '\n';

  std::vector<int> values{1, 2, 3};

  // decltype(auto) preserves the reference returned by front().
  get_first(values) = 42;

  std::cout << "values[0] = " << values[0] << '\n';

  // Trailing return type using decltype.
  std::cout << "add_decltype(3, 4.5) = " << add_decltype(3, 4.5) << '\n';
}

// ============================================================
// std::enable_if
// ============================================================
//
// enable_if conditionally provides a type.
//
// If the condition is true:
//
//     enable_if<true, T>::type
//
// exists.
//
// If the condition is false:
//
//     enable_if<false, T>::type
//
// does not exist.
//
// This was a common C++11/14 technique for constraining
// templates before concepts.
//
// Example:
//
// Give me type "T" ONLY if "T" is integral, otherwise function
// disappears:
//
//    std::is_integral<T>::value -> true/false
//    std::enable_if<cond, T>    -> gives type ONLY if cond == true
//    ::type                     -> extract the type
//    typename                   -> tell compiler it's a type
//
// Implementation:
//
//    template<bool, typename T>
//    struct enable_if {};
//
//    template<true, typename T>
//    struct enable_if<true, T> {
//      typedef T type;
//    };
// ============================================================

template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type square(T x) {
  return x * x;
}

// C++14 shorthand:
// std::enable_if_t<condition, T>
//
template <typename T>
std::enable_if_t<std::is_integral_v<T>, T> cube(T x) {
  return x * x * x;
}

void example_enable_if() {
  std::cout << "\n=== std::enable_if ===\n";

  std::cout << "square(5) = " << square(5) << '\n';

  std::cout << "cube(4) = " << cube(4) << '\n';

  // square(3.14);
  // ERROR:
  // enable_if condition is false, so the function template
  // is not a valid candidate for double.
}

// ============================================================
// SFINAE
// ============================================================
//
// SFINAE:
//
//     Substitution Failure Is Not An Error
//
// During template substitution, if forming a candidate template
// produces an invalid type/expression in the immediate context,
// that candidate is discarded instead of producing a hard
// compilation error.
//
// enable_if is one way to use SFINAE.
//
// void_t + decltype is another classic technique for detecting
// whether a type provides a particular member.
// ============================================================

template <typename T>
std::enable_if_t<std::is_integral_v<T>, T> increment(T value) {
  std::cout << "Integral version\n";
  return value + 1;
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, T> increment(T value) {
  std::cout << "Floating-point version\n";
  return value + 0.5;
}

// Detection idiom:
//
// Primary template: assume false.
template <typename, typename = void>
struct has_size_method : std::false_type {};

// If T::size() is valid, this specialization is selected.
//
// decltype(std::declval<T>().size())
//     -> type of T::size()
//
// std::void_t<...>
//     -> void if the expression is valid
//
// If the expression is invalid, substitution fails and the
// specialization is discarded by SFINAE.
template <typename T>
struct has_size_method<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type {};

void example_sfinae() {
  std::cout << "\n=== SFINAE ===\n";

  std::cout << increment(10) << '\n';
  std::cout << increment(10.0) << '\n';

  std::cout << std::boolalpha;

  // int has no size() member.
  std::cout << "int has size(): " << has_size_method<int>::value << '\n';

  // vector has size().
  std::cout << "vector has size(): " << has_size_method<std::vector<int>>::value
            << '\n';

  // string has size().
  std::cout << "string has size(): " << has_size_method<std::string>::value
            << '\n';

  // increment("hello");
  //
  // Neither integral nor floating-point overload is viable.
  // The invalid candidates are discarded through SFINAE.
}

// ============================================================
// CONCEPTS
// ============================================================
//
// Concepts were introduced in C++20 to express requirements on
// template arguments directly.
//
// Instead of indirectly expressing:
//
//     "T must support operator<"
//
// through SFINAE machinery, we can name the requirement.
//
// A concept is essentially a named compile-time constraint.
// ============================================================

template <typename T>
concept HasLessThan = requires(const T& a, const T& b) {
  { a < b } -> std::convertible_to<bool>;
};

template <typename T>
concept Printable = requires(const T& value) { std::cout << value; };

template <Printable T>
void print_value(const T& value) {
  std::cout << "printable value: " << value << '\n';
}

void example_concepts() {
  std::cout << "\n=== Concepts ===\n";

  print_value(42);
  print_value(3.14);
  print_value(std::string{"hello"});

  // HasLessThan is a named concept that can be reused
  // anywhere a comparable type is required.
  std::cout << std::boolalpha;

  std::cout << "int HasLessThan: " << HasLessThan<int> << '\n';

  std::cout << "string HasLessThan: " << HasLessThan<std::string> << '\n';
}

// ============================================================
// REQUIRES CLAUSES
// ============================================================
//
// A requires-clause constrains a template.
//
// Concepts and requires clauses are closely related:
//
//     concept
//         names a reusable constraint
//
//     requires
//         applies constraints / tests expressions
// ============================================================

template <typename T>
requires std::copyable<T> && HasLessThan<T>
T constrained_max(T a, T b) {
  return b < a ? a : b;
}

// A requires-clause can also combine standard concepts.
template <typename T>
requires std::integral<T> || std::floating_point<T>
T square_constrained(T x) {
  return x * x;
}

template <std::integral T>
void print_type(T) {
  std::cout << "Integral type\n";
}

template <std::floating_point T>
void print_type(T) {
  std::cout << "Floating-point type\n";
}

// Abbreviated function template.
//
// This:
//
//     void add(std::integral auto a, std::integral auto b)
//
// is an abbreviated way of expressing a constrained function
// template.
auto add_integral(std::integral auto a, std::integral auto b) { return a + b; }

// Concepts can constrain classes too.
template <std::totally_ordered T>
class Range {
 private:
  T low;
  T high;

 public:
  Range(T low, T high) : low(low), high(high) {}

  bool contains(const T& value) const { return value >= low && value <= high; }
};

void example_requires_clauses() {
  std::cout << "\n=== Requires Clauses ===\n";

  std::cout << "constrained_max: " << constrained_max(10, 20) << '\n';

  std::string a = "apple";
  std::string b = "banana";

  std::cout << "constrained_max(string): " << constrained_max(a, b) << '\n';

  std::cout << "square_constrained(int): " << square_constrained(5) << '\n';

  std::cout << "square_constrained(double): " << square_constrained(5.5)
            << '\n';

  std::cout << "print_type(int): ";
  print_type(42);

  std::cout << "print_type(double): ";
  print_type(4.2);

  std::cout << "add_integral: " << add_integral(10, 20) << '\n';

  Range<int> range(10, 20);

  std::cout << std::boolalpha << "15 in range: " << range.contains(15) << '\n';

  // These would fail their constraints:
  //
  // constrained_max(std::complex<double>{},
  //                 std::complex<double>{});
  //
  // square_constrained(std::string{"hello"});
  //
  // add_integral(1.0, 2.0);
}

// ============================================================
// CRTP
// ============================================================
//
// Curiously Recurring Template Pattern:
//
//     class Derived : public Base<Derived>
//
// The derived class passes itself as the template argument to
// the base class.
//
// This gives compile-time polymorphism rather than virtual
// dispatch.
//
// It is useful for mixins and static interfaces.
//
// https://eli.thegreenplace.net/2011/05/17/the-curiously-recurring-template-pattern-in-c/
// ============================================================

template <typename Derived>
class Logger {
 public:
  Logger() {
    std::cout << "[Ctor] " << static_cast<Derived*>(this)->to_string() << "\n";
  }

  ~Logger() {
    std::cout << "[Dtor] " << static_cast<Derived*>(this)->to_string() << "\n";
  }
};

// Counter mixin
template <typename Derived>
class Counter {
 private:
  static int count;

 public:
  Counter() { ++count; }
  Counter(const Counter&) { ++count; }
  Counter(Counter&&) noexcept { ++count; }
  Counter& operator=(const Counter& other) { ++count; }
  ~Counter() { --count; }
  static int get_count() { return count; }
};

template <typename Derived>
int Counter<Derived>::count = 0;

// Addable mixin
template <typename Derived>
class Addable {
 public:
  Derived operator+(const Derived& other) const {
    Derived result = static_cast<const Derived&>(*this);
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
    std::cout << static_cast<const Derived*>(this)->to_string() << "\n";
  }
};

// Derived class
class Money : public Entity<Money> {
 public:
  int value;

  Money(int v = 0) : value(v) {}

  // Required by Addable mixin
  Money operator+(const Money& other) {
    Money res = static_cast<const Money&>(*this);
    res.value += other.value;
    return res;
  }

  // Required by Logger mixin
  std::string to_string() const {
    return "Money (" + std::to_string(value) + ")";
  }

  int get_value() const { return value; }
};

void example_crtp() {
  std::cout << "\n=== CRTP ===\n";

  Money m1(30);
  Money m2(50);

  // operator+ is supplied by Addable<Money>.
  Money m3 = m1 + m2;

  m3.print();

  std::cout << "m3.value = " << m3.get_value() << '\n';

  {
    Money m4(100);
    std::cout << "Money instances (inside block): " << Money::get_count()
              << "\n";
  }

  std::cout << "Money instances (outside block): " << Money::get_count()
            << "\n";

  // No virtual function is involved.
  //
  // The base template knows at compile time that Derived = Money.
}

// ============================================================
// TEMPLATE METAPROGRAMMING BASICS
// ============================================================
//
// Templates can be used as a compile-time computation mechanism.
//
// This is the simple recursive style discussed by Josuttis:
//
//     Factorial<5>
//         -> Factorial<4>
//             -> ...
//                 -> Factorial<0>
//
// The specialization provides the terminating case.
// ============================================================

template <int N>
struct Factorial {
  static constexpr int value = N * Factorial<N - 1>::value;
};

template <>
struct Factorial<0> {
  static constexpr int value = 1;
};

void example_template_metaprogramming() {
  std::cout << "\n=== Template Metaprogramming Basics ===\n";

  constexpr int value = Factorial<5>::value;

  std::cout << "5! = " << value << '\n';

  static_assert(Factorial<5>::value == 120);

  // The calculation is known at compile time.
}

// ============================================================
// FOLD EXPRESSIONS
// ============================================================
//
// Fold expressions were introduced in C++17.
//
// They provide a concise way to reduce a parameter pack.
//
// Four forms:
//
//     (... op pack)         -> (a1 op a2 op a3)
//     (pack op ...)         -> (a1 op a2 op a3)
//
//     (init op ... op pack) -> (((init op a1) op a2) op a3)
//     (pack op ... op init) -> (a1 op (a2 op (a3 op init)))
//
// Examples below use:
//
//     (args + ...)
//
// and:
//
//     (0 + ... + args)
//
// Fold expressions often replace recursive variadic-template
// implementations.
// ============================================================

template <typename... Args>
auto sum(Args... args) {
  // Unary right fold.
  return (args + ...);
}

template <typename... Args>
auto sum_with_initial_value(Args... args) {
  // Binary left fold with initial value 0.
  //
  // Useful when the parameter pack can be empty.
  return (0 + ... + args);
}

template <typename... Args>
void print_fold(Args... args) {
  // The comma operator evaluates the expressions from
  // left to right.
  //
  // Conceptually:
  //
  //     ((cout << a << " "),
  //      (cout << b << " "),
  //      (cout << c << " "))
  //
  ((std::cout << args << ' '), ...);

  std::cout << '\n';
}

template <typename... Args>
std::string join_with_exclamation(Args... args) {
  return (args + ... + std::string("!\n"));
}

void example_fold_expressions() {
  std::cout << "\n=== Fold Expressions ===\n";

  std::cout << "sum = " << sum(1, 2, 3, 4) << '\n';

  std::cout << "sum with initial value = " << sum_with_initial_value(1, 2, 3, 4)
            << '\n';

  // Empty pack is valid for the binary fold because
  // the initial value is provided.
  std::cout << "empty sum = " << sum_with_initial_value() << '\n';

  print_fold(1, 2.5, "hello");

  std::cout << join_with_exclamation(
      std::string{"A"}, std::string{"B"}, std::string{"C"});
}

// ============================================================
// TYPE TRAITS
// ============================================================
//
// Type traits provide compile-time information about types.
//
// Common examples:
//
//     std::is_same
//     std::is_pointer
//     std::is_integral
//     std::is_floating_point
//
// The *_v forms are the C++17 shorthand:
//
//     std::is_pointer_v<T>
//
// instead of:
//
//     std::is_pointer<T>::value
// ============================================================

template <typename T>
void inspect_type() {
  if constexpr (std::is_pointer_v<T>) {
    std::cout << "Pointer\n";
  } else if constexpr (std::is_integral_v<T>) {
    std::cout << "Integral\n";
  } else if constexpr (std::is_floating_point_v<T>) {
    std::cout << "Floating point\n";
  } else {
    std::cout << "Other\n";
  }
}

void example_type_traits() {
  std::cout << "\n=== Type Traits ===\n";

  inspect_type<int>();
  inspect_type<double>();
  inspect_type<int*>();
  inspect_type<std::string>();

  std::cout << std::boolalpha;

  std::cout << "int == int: " << std::is_same_v<int, int> << '\n';

  std::cout << "int == double: " << std::is_same_v<int, double> << '\n';

  std::cout << "int is integral: " << std::is_integral_v<int> << '\n';

  std::cout << "double is floating point: "
            << std::is_floating_point_v<double> << '\n';
}

// ============================================================
// NTTP — NON-TYPE TEMPLATE PARAMETERS
// ============================================================
//
// Template parameters don't have to be types.
//
// They can also be values:
//
//     template <std::size_t N>
//
// Different values produce different template specializations:
//
//     FixedArray<int, 4>
//     FixedArray<int, 8>
//
// These are different C++ types.
// ============================================================

template <std::size_t N>
void repeat_hello() {
  for (std::size_t i = 0; i < N; ++i) {
    std::cout << "Hello!\n";
  }
}

// Array references are particularly useful for demonstrating
// NTTP deduction:
//
//     T = element type
//     N = array size
//
// The array does not decay to a pointer because it is bound to
// a reference.
template <typename T, std::size_t N>
void print_array(const T (&arr)[N]) {
  std::cout << "Array size = " << N << " => ";

  for (std::size_t i = 0; i < N; ++i) {
    std::cout << arr[i] << ' ';
  }

  std::cout << '\n';
}

void example_nttp() {
  std::cout << "\n=== NTTP ===\n";

  repeat_hello<2>();

  int arr[] = {1, 2, 3, 4};

  // T = int
  // N = 4
  //
  // N is deduced from the array itself.
  print_array(arr);

  FixedArray<int, 4> fixed_array;

  fixed_array.fill(10);

  std::cout << "FixedArray size = " << fixed_array.size() << '\n';
}

// ============================================================
// RUN ALL EXAMPLES
// ============================================================

int run() {
  example_function_templates();
  example_class_templates();

  example_template_specialization();
  example_partial_specialization();

  example_variadic_templates();
  example_template_type_deduction();

  example_decltype();
  example_enable_if();
  example_sfinae();

  example_concepts();
  example_requires_clauses();

  example_crtp();
  example_template_metaprogramming();

  example_fold_expressions();
  example_type_traits();

  example_nttp();

  return 0;
}

}  // namespace TP
