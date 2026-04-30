#include <array>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace MD {

// ------------
// Move Semantics
// ------------

class Buffer {
 public:
  Buffer(size_t n) : size(n) {
    std::cout << "[Ctor] size=" << n << "\n";
    data = new char[n];
    std::memset(data, 0, n);
  }

  ~Buffer() {
    std::cout << "[Dtor] size=" << size << "\n";
    delete[] data;
  }

  // copy constructor
  Buffer(const Buffer& other) : size(other.size) {
    std::cout << "[Copy Ctor] size=" << size << "\n";
    data = new char[other.size];
    std::memcpy(data, other.data, size);
  }

  // move constructor
  //
  // |noexcept|: tells compiler that a function will not throw an exception
  //   - enable compiler optimization
  //   - allow STL containers to use move instead of copy constructor during
  //     reallocation
  Buffer(Buffer&& other) noexcept : data(other.data), size(other.size) {
    std::cout << "[Move Ctor] size=" << size << "\n";
    other.data = nullptr;
    other.size = 0;
  }

  // copy assign
  Buffer& operator=(const Buffer& other) {
    std::cout << "[Copy Assign] size=" << other.size << "\n";

    if (this != &other) {
      delete[] data;
      size = other.size;
      data = new char[size];
      std::memcpy(data, other.data, size);
    }

    return *this;
  }

  // move assign
  Buffer& operator=(Buffer&& other) noexcept {
    std::cout << "[Move Assign] size=" << other.size << "\n";

    if (this != &other) {
      delete[] data;
      size = other.size;
      data = other.data;

      other.size = 0;
      other.data = nullptr;
    }

    return *this;
  }

  void print() const { std::cout << "size=" << size << "\n"; }

 private:
  char* data;
  size_t size;
};

void test_move_semantics() {
  Buffer a(100);
  // std::move: only cast to rvalue, treat |a| as movable
  Buffer b = std::move(a);
  Buffer c = Buffer(200);

  a.print();
  b.print();
  c.print();
}

// -----------
// perfect forwarding
// -----------
//
// Used in wrapper/template functions (e.g., emplace_back), preserves:
//   - lvalue/rvalue nature
//   - constness
//   - references
//
void process(const std::string& s) {
  std::cout << "process(const&): " << s << "\n";
}

void process(std::string&& s) { std::cout << "process(&&): " << s << "\n"; }

template <typename T>
void wrapper(T&& arg) {
  std::cout << "wrapper forwarding...\n";
  process(std::forward<T>(arg));  // perfect forwarding
}

template <typename T>
class MyVector {
  T* data;
  size_t capacity;
  size_t pos;

 public:
  MyVector(int cap = 4) : capacity(cap), pos(0) {
    // ::operator new(sizeof(T) * capacity): allocate raw memory like malloc
    data = static_cast<T*>(::operator new(sizeof(T) * capacity));
  }

  ~MyVector() {
    for (size_t i = 0; i < pos; ++i) {
      data[i].~T();
    }
    // ::operator delete(data): free raw memory
    ::operator delete(data);
  }

  void push_back(const T& value) {
    std::cout << "push_back(const T&) called\n";
    // new (ptr) T(args...): placement new, constructs objects at that memory
    new (&data[pos++]) T(value);
  }

  void push_back(T&& value) {
    std::cout << "push_back(T&&) called\n";
    new (&data[pos++]) T(std::move(value));
  }

  template <typename... Args>
  void emplace_back(Args&&... args) {
    std::cout << "emplace_back(Args&&...) called\n";
    new (&data[pos++]) T(std::forward<Args>(args)...);
  }

  const T& operator[](size_t i) const { return data[i]; }

  size_t size() const { return pos; }
};

void test_perfect_forwarding() {
  std::cout << "--- Basic ---\n";
  std::string name = "Hello";
  wrapper(name);                  // lvalue
  wrapper(std::string("World"));  // rvalue

  std::cout << "--- emplace_back and push_back ---\n";
  MyVector<Buffer> mv1;
  Buffer b1(300);
  mv1.push_back(b1);  // copy

  MyVector<Buffer> mv2;
  mv2.push_back(Buffer(400));  // move

  MyVector<Buffer> mv3;
  mv3.emplace_back(500);  // in-place, no copy/move
}

// -----------
// move/forward
// -----------
//
// move: always converts to rvalue
// forward<T>: conditionally preserve original category
//
// **no example code**

// -----------
// Rvalue Reference (T&&)
// -----------

// binds to temporary/movable objects, usually used in:
//   - move constructor
//   - move assignment
//   - forwarding references
//
// IMPORTANT: named rvalue reference becomes lvalue

void wrapper2(std::string&& s) {
  process(s);             // lvalue;
  process(std::move(s));  // rvalue;
}

void test_rvalue_reference() { wrapper2("temp string"); }

// -----------
// Uniform Initialization {}
// -----------
//
// Search order:
//   1. initializer_list constructor
//   2. regular constructor
//   3. aggregate initializer

// Aggregate class or struct
class Dog {
 public:
  int age;
  std::string name;
};

class Person {
 public:
  int age;  // 3rd choice
  std::string name;

  Person(int a, std::string n) : age(a), name(n) {  // 2nd choice
    std::cout << "2nd choice\n";
  }

  Person(const std::initializer_list<int>& vec) {  // 1st choice
    std::cout << "1st choice\n";
    age = *(vec.begin());
  }
};

void test_init() {
  // =
  {
    int x = 5;
    // int y = 3.14; // allow narrowing conversion, warning
    std::string a = "hello";
    std::cout << "=: " << x << " " << a << "\n";
  }

  // (): calls a constructor
  {
    int x(5);
    // int y(3.14); // allow narrowing conversion, warning
    std::string a("hello");
    std::cout << "(): " << x << " " << a << "\n";
  }

  // {}: c++11, uniform initialization
  {
    int x{5};
    // int y{3.14}; // ERROR: narrowing double -> int
    std::string a{"hello"};

    int arr[] = {2, 3, 4};             // c++ 03 initializer list
    std::vector<int> vec = {3, 4, 5};  // c++ 11 extended the support to all
                                       // relevant STL containers
    int arr2[]{2, 3, 4};
    std::vector<int> vec2{3, 4, 5};

    std::cout << "{}: " << x << " " << a << " " << arr[0] << " " << vec[0]
              << " " << arr2[0] << " " << vec2[0] << "\n";
  }

  Dog d{20, "Boba"};
  Person p1{10};
  Person p2{20, "Bob"};
}

// -----------
// constexpr functions
// -----------
//
// IMPORTANT:
//   - zero runtime cost
//   - compile-time tables
//   - metaprogramming
//   - validation

constexpr int square(int x) { return x * x; }

void test_constexpr_function() {
  constexpr int n = square(5);
  std::array<int, n> arr{};
  std::cout << "array variable declared successfully\n";
}

// -----------
// if constexpr
// -----------
template <typename T>
void print_info(const T& value) {
  if constexpr (std::is_arithmetic_v<T>) {
    std::cout << "Arithmetic value: " << value << "\n";
  } else if constexpr (std::is_same_v<T, std::string>) {
    std::cout << "String value: " << value << "\n";
  } else {
    std::cout << "Other type" << "\n";
  }
}

void test_if_constexpr() {
  print_info(42);
  print_info(3.14);
  print_info(std::string("Hi"));
  print_info(std::vector<int>{1, 2, 3});
}
// -----------
// modules
// -----------
//
// Replacement for headers
//
// **no example code**

// -----------
// Coroutines
// -----------
//
// Functions that can suspend/resume, used for
//   - async IO
//   - generators
//   - task systems
//
// **no example code**

// -----------
// std::span
// -----------
//
// Non-owning view over contiguous memory, can view:
//   - array
//   - vector
//   - raw pointer + size

void print(std::span<int> s) {
  for (int x : s) {
    std::cout << x << " ";
  }
  std::cout << "\n";
}

void test_span() {
  std::vector<int> v{1, 2, 3};
  int arr[]{4, 5, 6};

  print(v);
  print(arr);
}

// -----------
// Designated Initializers
// -----------
//
struct Config {
  int threads;
  int port;
};

Config c{.threads = 8, .port = 8080};

int run() {
  std::cout << "=== Move Semantics ===\n";
  test_move_semantics();

  std::cout << "=== Perfect Forwarding ===\n";
  test_perfect_forwarding();

  std::cout << "=== Rvalue Reference ===\n";
  test_rvalue_reference();

  std::cout << "=== Uniform Initialization ===\n";
  test_init();

  std::cout << "=== constexpr function ===\n";
  test_constexpr_function();

  std::cout << "=== if constexpr ===\n";
  test_if_constexpr();

  std::cout << "=== std::span ===\n";
  test_span();

  return 0;
}

}  // namespace MD
