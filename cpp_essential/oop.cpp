#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

namespace OOP {

//////////////////////////////////////////////////////////////
// Real-world pitfalls shown below
// ❌ forgetting virtual destructor → UB
// ❌ copying polymorphic objects → slicing
// ❌ shallow copy of owning pointer → double free
// ❌ not implementing clone → broken deep copy
// ❌ forgetting noexcept move → STL falls back to copy
// ❌ using raw pointers in container → leaks
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// (21–27) Resource class to force Rule of 5
//////////////////////////////////////////////////////////////

class Buffer {
 private:
  char* data;
  size_t size;

 public:
  Buffer(size_t sz) : size(sz), data(new char[sz]) {
    std::cout << "[Buffer ctor]\n";
    // void* memset(void* dest, int ch, size_t count);
    std::memset(data, 0, size);
  }

  ~Buffer() {
    std::cout << "[Buffer dtor]\n";
    delete[] data;
  }

  // copy ctor
  Buffer(const Buffer& other) : size(other.size), data(new char[other.size]) {
    std::cout << "[Buffer copy ctor]\n";
    std::memcpy(data, other.data, size);
  }

  // copy assign
  Buffer& operator=(const Buffer& other) {
    std::cout << "[Buffer copy assign]\n";
    if (this == &other) return *this;

    delete[] data;
    size = other.size;
    data = new char[size];
    std::memcpy(data, other.data, size);

    return *this;
  }

  // move ctor
  Buffer(Buffer&& other) noexcept : data(other.data), size(other.size) {
    std::cout << "[Buffer move ctor]\n";
    other.data = nullptr;
    other.size = 0;
  }

  // move assign
  Buffer& operator=(Buffer&& other) noexcept {
    std::cout << "[Buffer move assign]\n";
    if (this == &other) return *this;

    delete[] data;
    data = other.data;
    size = other.size;

    other.data = nullptr;
    other.size = 0;

    return *this;
  }

  void write(const char* new_data) {
    // char* strncpy(char* dest, const char* src, size_t count);
    // NOTE: the last char is \0 - null terminator for valid C string
    std::strncpy(data, new_data, size - 1);
  }

  const char* c_str() const { return data; }
};

//////////////////////////////////////////////////////////////
// (21–35) Base Class
//////////////////////////////////////////////////////////////

class Item {
 protected:
  int id;

  // (29) friend
  friend class Inventory;

 public:
  static int global_item_count;  // (28) static member

  Item(int id_) : id(id_) {
    std::cout << "[Item ctor]\n";
    ++global_item_count;
  }

  // Must be virtual
  virtual ~Item() {
    std::cout << "[Item dtor]\n";
    --global_item_count;
  }

  // (32, 33) polymorphism
  virtual void use() const = 0;  // abstract class

  virtual std::unique_ptr<Item> clone() const = 0;

  int GetID() const { return id; }
};

int Item::global_item_count = 0;

//////////////////////////////////////////////////////////////
// Derived: Weapon
//////////////////////////////////////////////////////////////

class Weapon : public Item {
 private:
  Buffer name;  // resource-owning -> triggers Rule of 5 behavior
  int damage;

 public:
  Weapon(int id, const char* name_, int damage_)
      : Item(id), name(32), damage(damage_) {
    std::cout << "[Weapon ctor]\n";
    name.write(name_);
  }

  ~Weapon() { std::cout << "[Weapon dtor]\n"; }

  void use() const override {
    std::cout << "Swing weapon: " << name.c_str() << " damage=" << damage
              << "\n";
  }

  std::unique_ptr<Item> clone() const override {
    return std::make_unique<Weapon>(*this);  // copy
  }

  // (30) operator overloading
  // TODO: return type
  Weapon operator+(const Weapon& other) const {
    return Weapon(id, name.c_str(), damage + other.damage);
  }

  // (29) friend
  friend std::ostream& operator<<(std::ostream& os, const Weapon& w);
};

std::ostream& operator<<(std::ostream& os, const Weapon& w) {
  os << "Weapon(damage=" << w.damage << "\n";
  return os;
}

//////////////////////////////////////////////////////////////
// Derived: Potion
//////////////////////////////////////////////////////////////

class Potion final : public Item {  // (35) final
 private:
  int heal;

 public:
  Potion(int id, int h) : Item(id), heal(h) { std::cout << "[Potion ctor]\n"; }

  ~Potion() { std::cout << "Potion dtor]\n"; }

  void use() const override {
    std::cout << "Drink potion: +" << heal << " HP\n";
  }

  std::unique_ptr<Item> clone() const override {
    return std::make_unique<Potion>(*this);
  }
};

//////////////////////////////////////////////////////////////
// (27,28,29) Inventory system
//////////////////////////////////////////////////////////////

class Inventory {
 private:
  std::vector<std::unique_ptr<Item>> items;

 public:
  Inventory() = default;

  // deep copy via clone
  Inventory(const Inventory& other) {
    std::cout << "[Inventory copy ctor]\n";
    for (const auto& it : other.items) {
      items.push_back(it->clone());
    }
  }

  Inventory& operator=(const Inventory& other) {
    std::cout << "[Inventory copy assign]\n";
    if (this == &other) return *this;

    items.clear();
    for (const auto& it : other.items) {
      items.push_back(it->clone());
    }

    return *this;
  }

  void add(std::unique_ptr<Item> item) {
    items.push_back(std::move(item));  // move semantics
  }

  void UseAll() const {
    for (const auto& it : items) {
      it->use();
    }
  }

  size_t size() const { return items.size(); }
};

//////////////////////////////////////////////////////////////
// (34) Object slicing demonstration
//////////////////////////////////////////////////////////////

/*
void badUse(Item item) {  // ⚠️ slicing
  std::cout << "[badUse]\n";
  item.use();
}
*/

void goodUse(const Item& item) {
  std::cout << "[goodUse]\n";
  item.use();
}

//////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////

int run() {
  std::cout << "\n--- Create Inventory ---\n";
  Inventory inv;

  inv.add(std::make_unique<Weapon>(1, "Sword", 10));
  inv.add(std::make_unique<Potion>(2, 25));

  std::cout << "\n--- Use Items ---\n";
  inv.UseAll();

  std::cout << "\n--- Copy Inventory (deep copy) ---\n";
  Inventory inv2 = inv;
  inv2.UseAll();

  std::cout << "\n--- Operator Overload ---\n";
  Weapon w1(3, "Axe", 15);
  Weapon w2(4, "Fire", 20);
  Weapon w3 = w1 + w2;
  w3.use();

  std::cout << "\n--- Slicing Demo ---\n";
  Weapon w4(5, "Dagger", 5);
  // badUse(w4);   // slicing
  goodUse(w4);  // correct

  std::cout << "\n--- Static Count ---\n";
  std::cout << "Item count: " << Item::global_item_count << "\n";

  std::cout << "\n--- End ---\n";
}

}  // namespace OOP
