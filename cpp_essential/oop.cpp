#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace OOP {

// ============================================================
// 21. Classes & structs
// ============================================================
//
// class and struct are almost identical in C++.
//
// The main difference:
//   class  -> members are private by default
//   struct -> members are public by default
//
// Otherwise they support:
//   - constructors
//   - methods
//   - static members
//   - inheritance
//   - virtual functions
//   - etc.
// ============================================================

class Player {
 private:
  std::string name;
  int level;

 public:
  Player(std::string name_, int level_)
      : name(std::move(name_)), level(level_) {}

  void print() const {
    std::cout << "Player: " << name << ", level=" << level << '\n';
  }
};

struct Position {
  float x;
  float y;

  void print() const { std::cout << "Position: (" << x << ", " << y << ")\n"; }
};

void example_21() {
  std::cout << "\n=== 21. Classes & structs ===\n";

  Player player("Alice", 10);
  player.print();

  Position position{10.0f, 20.0f};
  position.print();

  // Position members are public because Position is a struct.
  position.x = 30.0f;

  // Player's members are private because Player is a class.
  // player.level = 20;  // ERROR
}

// ============================================================
// 22. Access specifiers
// ============================================================
//
// public:
//   accessible from anywhere
//
// protected:
//   accessible from the class and derived classes
//
// private:
//   accessible only from the class itself
//
// Note:
// "private" means class-private, not object-private.
// A member function can access private members of another
// object of the same class.
// ============================================================

class BankAccount {
 private:
  double balance = 0.0;

 protected:
  void add_bonus(double amount) { balance += amount; }

 public:
  BankAccount(double initial_balance) : balance(initial_balance) {}

  void deposit(double amount) { balance += amount; }

  double get_balance() const { return balance; }
};

class PremiumAccount : public BankAccount {
 public:
  PremiumAccount(double initial_balance) : BankAccount(initial_balance) {}

  void apply_bonus() {
    // protected member is accessible here.
    add_bonus(100.0);
  }

  // balance is private in BankAccount,
  // so this would NOT compile:
  //
  // balance += 100;
};

void example_22() {
  std::cout << "\n=== 22. Access specifiers ===\n";

  PremiumAccount account(1000.0);

  account.deposit(500.0);
  account.apply_bonus();

  std::cout << "Balance: " << account.get_balance() << '\n';

  // account.balance = 999999;  // ERROR: private
  // account.add_bonus(100);    // ERROR: protected
}

// ============================================================
// 23. Constructors / destructors
// ============================================================
//
// Constructor:
//   initializes an object.
//
// Destructor:
//   runs when an object is destroyed.
//
// Destruction happens:
//   - when a local object leaves scope
//   - when delete is called
//   - when an owning smart pointer is destroyed
//   - when a containing object is destroyed
//
// Member initialization order is:
//   1. base classes
//   2. data members, in declaration order
//   3. constructor body
// ============================================================

class Logger {
 public:
  Logger() { std::cout << "Logger constructed\n"; }

  ~Logger() { std::cout << "Logger destroyed\n"; }

  void log(const std::string& message) const { std::cout << message << '\n'; }
};

class Application {
 private:
  Logger logger;

 public:
  Application() { std::cout << "Application constructed\n"; }

  ~Application() { std::cout << "Application destroyed\n"; }
};

void example_23() {
  std::cout << "\n=== 23. Constructors / destructors ===\n";

  {
    Application app;
    // app.logger;  // ERROR if uncommented: private
  }

  std::cout << "Scope ended\n";
}

// ============================================================
// 24. Copy constructor / copy assignment
// ============================================================
//
// Copy constructor:
//   creates a NEW object from an existing object.
//
//     Widget b = a;
//     Widget b(a);
//
// Copy assignment:
//   assigns to an EXISTING object.
//
//     b = a;
//
// These are fundamentally different operations.
// ============================================================

class Widget {
 private:
  std::string name;

 public:
  Widget(std::string name_) : name(std::move(name_)) {
    std::cout << "[Widget constructor]\n";
  }

  // Copy constructor
  Widget(const Widget& other) : name(other.name) {
    std::cout << "[Widget copy constructor]\n";
  }

  // Copy assignment
  Widget& operator=(const Widget& other) {
    std::cout << "[Widget copy assignment]\n";

    if (this == &other) {
      return *this;
    }

    name = other.name;
    return *this;
  }

  void print() const { std::cout << "Widget: " << name << '\n'; }
};

void example_24() {
  std::cout << "\n=== 24. Copy constructor / copy assignment ===\n";

  Widget a("Sword");

  // New object -> copy constructor
  Widget b = a;

  // New object -> copy constructor
  Widget c(a);

  // Existing object -> copy assignment
  c = b;

  a.print();
  b.print();
  c.print();
}

// ============================================================
// 25. Move constructor / move assignment
// ============================================================
//
// Moving transfers resources/state from an object instead of
// making an expensive copy.
//
// Move constructor:
//     T b = std::move(a);
//
// Move assignment:
//     b = std::move(a);
//
// std::move() itself does NOT move anything.
// It converts its argument into an rvalue so that a move
// operation can be selected.
// ============================================================

class Buffer {
 private:
  std::unique_ptr<int[]> data;
  size_t size;

 public:
  explicit Buffer(size_t size_)
      : data(std::make_unique<int[]>(size_)), size(size_) {
    std::cout << "[Buffer constructor]\n";
  }

  // Copy constructor
  Buffer(const Buffer& other)
      : data(std::make_unique<int[]>(other.size)), size(other.size) {
    std::cout << "[Buffer copy constructor]\n";

    for (size_t i = 0; i < size; ++i) {
      data[i] = other.data[i];
    }
  }

  // Move constructor
  Buffer(Buffer&& other) noexcept
      : data(std::move(other.data)), size(other.size) {
    std::cout << "[Buffer move constructor]\n";

    other.size = 0;
  }

  // Copy assignment
  Buffer& operator=(const Buffer& other) {
    std::cout << "[Buffer copy assignment]\n";

    if (this == &other) {
      return *this;
    }

    auto new_data = std::make_unique<int[]>(other.size);

    for (size_t i = 0; i < other.size; ++i) {
      new_data[i] = other.data[i];
    }

    data = std::move(new_data);
    size = other.size;

    return *this;
  }

  // Move assignment
  Buffer& operator=(Buffer&& other) noexcept {
    std::cout << "[Buffer move assignment]\n";

    if (this == &other) {
      return *this;
    }

    data = std::move(other.data);
    size = other.size;

    other.size = 0;

    return *this;
  }

  size_t get_size() const { return size; }
};

void example_25() {
  std::cout << "\n=== 25. Move constructor / move assignment ===\n";

  Buffer a(100);

  // Copy
  Buffer b = a;

  // Move constructor
  Buffer c = std::move(a);

  std::cout << "a size after move: " << a.get_size() << '\n';

  Buffer d(50);

  // Move assignment
  d = std::move(c);

  std::cout << "c size after move: " << c.get_size() << '\n';
}

// ============================================================
// 26. Rule of 3 / 5 / 0
// ============================================================
//
// Rule of 3:
// If a class manually manages a resource and needs one of:
//
//   destructor
//   copy constructor
//   copy assignment
//
// it probably needs all three.
//
// Rule of 5:
// In modern C++, also consider:
//
//   move constructor
//   move assignment
//
// Rule of 0:
// Prefer designing the class so that it doesn't manually manage
// resources at all.
//
// Use RAII types such as:
//   std::string
//   std::vector
//   std::unique_ptr
//   std::shared_ptr
//
// Then the compiler-generated special members are usually enough.
// ============================================================

// -------------------------
// Rule of 5 example
// -------------------------

class RawBuffer {
 private:
  size_t size;
  int* data;

 public:
  explicit RawBuffer(size_t size_) : size(size_), data(new int[size_]) {
    std::cout << "[RawBuffer constructor]\n";
  }

  ~RawBuffer() {
    std::cout << "[RawBuffer destructor]\n";
    delete[] data;
  }

  // Copy constructor
  RawBuffer(const RawBuffer& other)
      : size(other.size), data(new int[other.size]) {
    std::cout << "[RawBuffer copy constructor]\n";

    for (size_t i = 0; i < size; ++i) {
      data[i] = other.data[i];
    }
  }

  // Copy assignment
  RawBuffer& operator=(const RawBuffer& other) {
    std::cout << "[RawBuffer copy assignment]\n";

    if (this == &other) {
      return *this;
    }

    int* new_data = new int[other.size];

    for (size_t i = 0; i < other.size; ++i) {
      new_data[i] = other.data[i];
    }

    delete[] data;

    data = new_data;
    size = other.size;

    return *this;
  }

  // Move constructor
  RawBuffer(RawBuffer&& other) noexcept : size(other.size), data(other.data) {
    std::cout << "[RawBuffer move constructor]\n";

    other.size = 0;
    other.data = nullptr;
  }

  // Move assignment
  RawBuffer& operator=(RawBuffer&& other) noexcept {
    std::cout << "[RawBuffer move assignment]\n";

    if (this == &other) {
      return *this;
    }

    delete[] data;

    size = other.size;
    data = other.data;

    other.size = 0;
    other.data = nullptr;

    return *this;
  }
};

// -------------------------
// Rule of 0 example
// -------------------------

class SafeBuffer {
 private:
  std::vector<int> data;

 public:
  explicit SafeBuffer(size_t size) : data(size) {}

  // No destructor
  // No copy constructor
  // No copy assignment
  // No move constructor
  // No move assignment
  //
  // std::vector handles its own resource management.
};

void example_26() {
  std::cout << "\n=== 26. Rule of 3 / 5 / 0 ===\n";

  std::cout << "--- Rule of 5 ---\n";

  RawBuffer a(10);
  RawBuffer b = a;
  RawBuffer c = std::move(a);

  b = c;
  c = std::move(b);

  std::cout << "--- Rule of 0 ---\n";

  SafeBuffer x(100);
  SafeBuffer y = x;
  SafeBuffer z = std::move(y);

  std::cout << "Rule of 0 needs no manually-written "
               "special member functions.\n";
}

// ============================================================
// 27. this pointer
// ============================================================
//
// Every non-static member function has access to:
//
//     this
//
// `this` is a pointer to the object on which the member function
// was called.
//
// For:
//
//     object.foo();
//
// inside foo():
//
//     this == &object
// ============================================================

class Counter {
 private:
  int value;

 public:
  Counter(int value) : value(value) {}

  Counter& increment() {
    ++this->value;

    // Returning *this allows chaining.
    return *this;
  }

  void print() const { std::cout << "Counter: " << this->value << '\n'; }

  bool is_same_object(const Counter& other) const { return this == &other; }
};

void example_27() {
  std::cout << "\n=== 27. this pointer ===\n";

  Counter a(0);
  Counter b(10);

  a.increment().increment().increment();

  a.print();

  std::cout << "a is same as a: " << a.is_same_object(a) << '\n';

  std::cout << "a is same as b: " << a.is_same_object(b) << '\n';
}

// ============================================================
// 28. Static members
// ============================================================
//
// A normal member belongs to each object.
//
// A static data member belongs to the CLASS.
//
// There is only one shared counter below, regardless of how
// many Player objects exist.
//
// A static member function has no `this` pointer because it is
// associated with the class rather than an object.
// ============================================================

class Connection {
 private:
  static int active_connections;

 public:
  Connection() { ++active_connections; }

  ~Connection() { --active_connections; }

  static int get_active_count() { return active_connections; }
};

// Definition required for a non-inline static data member.
int Connection::active_connections = 0;

void example_28() {
  std::cout << "\n=== 28. Static members ===\n";

  std::cout << "Connections: " << Connection::get_active_count() << '\n';

  Connection a;
  Connection b;

  std::cout << "Connections: " << Connection::get_active_count() << '\n';

  {
    Connection c;

    std::cout << "Connections: " << Connection::get_active_count() << '\n';
  }

  std::cout << "Connections: " << Connection::get_active_count() << '\n';
}

// ============================================================
// 29. Friend functions / friend classes
// ============================================================
//
// A friend is NOT a member of the class.
//
// But the class explicitly grants it access to private/protected
// members.
//
// Friendship is:
//   - explicit
//   - not inherited
//   - not transitive
//
// Common use:
//   operator<<
//   tightly-coupled helper functions
// ============================================================

class Order {
 private:
  int id;
  double price;

 public:
  Order(int id_, double price_) : id(id_), price(price_) {}

  friend std::ostream& operator<<(std::ostream& os, const Order& order);
};

// Friend function.
// It can access private members.
std::ostream& operator<<(std::ostream& os, const Order& order) {
  os << "Order{id=" << order.id << ", price=" << order.price << "}";

  return os;
}

void example_29() {
  std::cout << "\n=== 29. Friend functions / classes ===\n";

  Order order(42, 99.5);

  std::cout << order << '\n';
}

// ============================================================
// 30. Operator overloading
// ============================================================
//
// Operators can be overloaded for user-defined types.
//
// Example:
//
//     Position c = a + b;
//
// becomes conceptually:
//
//     operator+(a, b)
//
// Operators can be implemented as:
//   - member functions
//   - non-member functions
//   - friend functions
//
// Don't overload operators in surprising ways.
// ============================================================

class Money {
 private:
  int cents;

 public:
  explicit Money(int cents_) : cents(cents_) {}

  // Member operator
  Money operator+(const Money& other) const {
    return Money(cents + other.cents);
  }

  Money& operator+=(const Money& other) {
    cents += other.cents;
    return *this;
  }

  bool operator==(const Money& other) const { return cents == other.cents; }

  int get_cents() const { return cents; }
};

std::ostream& operator<<(std::ostream& os, const Money& money) {
  os << "$" << money.get_cents() / 100 << "." << money.get_cents() % 100;

  return os;
}

void example_30() {
  std::cout << "\n=== 30. Operator overloading ===\n";

  Money a(1000);  // $10
  Money b(250);   // $2.50

  Money c = a + b;

  std::cout << "a + b = " << c << '\n';

  c += a;

  std::cout << "c += a -> " << c << '\n';

  std::cout << "c == a: " << (c == a) << '\n';
}

// ============================================================
// 31. Inheritance basics
// ============================================================
//
// Inheritance expresses an "is-a" relationship.
//
// Weapon IS an Item.
// Potion IS an Item.
//
// A derived object contains a base-class subobject.
//
// Construction:
//
//     Base constructor
//     Derived constructor
//
// Destruction:
//
//     Derived destructor
//     Base destructor
// ============================================================

class Item {
 protected:
  int id;

 public:
  Item(int id_) : id(id_) { std::cout << "Item constructor\n"; }

  ~Item() { std::cout << "Item destructor\n"; }

  int get_id() const { return id; }
};

class Weapon : public Item {
 private:
  int damage;

 public:
  Weapon(int id_, int damage_) : Item(id_), damage(damage_) {
    std::cout << "Weapon constructor\n";
  }

  ~Weapon() { std::cout << "Weapon destructor\n"; }

  void attack() const {
    std::cout << "Weapon attacks for " << damage << " damage\n";
  }
};

void example_31() {
  std::cout << "\n=== 31. Inheritance basics ===\n";

  Weapon weapon(10, 50);

  std::cout << "ID: " << weapon.get_id() << '\n';

  weapon.attack();
}

// ============================================================
// 32. Virtual functions & polymorphism
// ============================================================
//
// Polymorphism means we can use a base-class interface while
// operating on different derived-class objects.
//
// Without virtual:
//
//     base_ptr->use();
//
// would select the Base implementation based on the STATIC
// type.
//
// With virtual:
//
//     base_ptr->use();
//
// selects the implementation based on the DYNAMIC type.
//
// This is dynamic dispatch.
// ============================================================

class Shape {
 public:
  virtual ~Shape() = default;

  virtual void draw() const { std::cout << "Drawing generic shape\n"; }
};

class Circle : public Shape {
 public:
  void draw() const override { std::cout << "Drawing circle\n"; }
};

class Rectangle : public Shape {
 public:
  void draw() const override { std::cout << "Drawing rectangle\n"; }
};

void draw_shape(const Shape& shape) { shape.draw(); }

void example_32() {
  std::cout << "\n=== 32. Virtual functions & polymorphism ===\n";

  Circle circle;
  Rectangle rectangle;

  draw_shape(circle);
  draw_shape(rectangle);

  std::vector<std::unique_ptr<Shape>> shapes;

  shapes.push_back(std::make_unique<Circle>());
  shapes.push_back(std::make_unique<Rectangle>());

  for (const auto& shape : shapes) {
    shape->draw();
  }
}

// ============================================================
// 33. Abstract classes / pure virtual functions
// ============================================================
//
// A class containing at least one pure virtual function is
// abstract.
//
//     virtual void draw() const = 0;
//
// You cannot instantiate an abstract class.
//
// Its purpose is to define an interface for derived classes.
// ============================================================

class Payment {
 public:
  virtual ~Payment() = default;

  virtual void pay(double amount) = 0;
};

class CreditCardPayment : public Payment {
 public:
  void pay(double amount) override {
    std::cout << "Paid $" << amount << " using credit card\n";
  }
};

class CryptoPayment : public Payment {
 public:
  void pay(double amount) override {
    std::cout << "Paid $" << amount << " using cryptocurrency\n";
  }
};

void example_33() {
  std::cout << "\n=== 33. Abstract classes ===\n";

  // Payment payment(100);  // ERROR: abstract class

  std::unique_ptr<Payment> payment = std::make_unique<CreditCardPayment>();

  payment->pay(100.0);

  payment = std::make_unique<CryptoPayment>();

  payment->pay(50.0);
}

// ============================================================
// 34. Object slicing
// ============================================================
//
// A derived object can be copied into a base object:
//
//     Base base = derived;
//
// Only the Base portion is copied.
//
// The Derived portion is sliced away.
//
// This is why polymorphic objects are normally passed by:
//
//     Base&
//     const Base&
//     Base*
//     std::unique_ptr<Base>
//     std::shared_ptr<Base>
//
// rather than by Base value.
// ============================================================

class Animal {
 public:
  virtual ~Animal() = default;

  virtual void speak() const { std::cout << "Animal sound\n"; }
};

class Dog : public Animal {
 public:
  void speak() const override { std::cout << "Woof!\n"; }
};

void take_by_value(Animal animal) { animal.speak(); }

void take_by_reference(const Animal& animal) { animal.speak(); }

void example_34() {
  std::cout << "\n=== 34. Object slicing ===\n";

  Dog dog;

  std::cout << "Passing by value:\n";
  take_by_value(dog);

  std::cout << "Passing by reference:\n";
  take_by_reference(dog);

  // Conceptually:
  //
  // Animal copy = dog;
  //
  // creates a completely separate Animal object.
  // The Dog-specific part no longer exists.
}

// ============================================================
// 35. override / final
// ============================================================
//
// override:
//
//     tells the compiler:
//
//     "This function MUST override a virtual function in a base."
//
// This catches mistakes such as:
//     wrong parameter type
//     wrong const qualifier
//     typo in function name
//
// final on a virtual function:
//
//     prevents further overriding.
//
// final on a class:
//
//     prevents inheritance from that class.
// ============================================================

class Engine {
 public:
  virtual ~Engine() = default;

  virtual void start() const { std::cout << "Engine starting\n"; }
};

class ElectricEngine : public Engine {
 public:
  void start() const override { std::cout << "Electric engine starting\n"; }
};

class TeslaEngine final : public ElectricEngine {
 public:
  void start() const final { std::cout << "Tesla engine starting\n"; }
};

// This would fail:
//
// class BetterTesla : public TeslaEngine {};
//
// TeslaEngine is final.
//
// This would also fail:
//
// class BrokenEngine : public Engine {
// public:
//     void Start() const override;
// };
//
// `Start` does not override `start` because C++ is
// case-sensitive.
//
// Without `override`, this typo would silently create
// a new function instead of overriding the base function.

void example_35() {
  std::cout << "\n=== 35. override / final ===\n";

  std::unique_ptr<Engine> engine = std::make_unique<ElectricEngine>();

  engine->start();

  engine = std::make_unique<TeslaEngine>();

  engine->start();

  std::cout << "override protects against accidental "
               "non-overrides.\n";

  std::cout << "final prevents further overriding/inheritance.\n";
}

// ============================================================
// Main
// ============================================================

void run() {
  example_21();
  example_22();
  example_23();
  example_24();
  example_25();
  example_26();
  example_27();
  example_28();
  example_29();
  example_30();
  example_31();
  example_32();
  example_33();
  example_34();
  example_35();
}

}  // namespace OOP
