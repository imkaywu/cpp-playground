#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace MM {

// ============================================================
// 1. STACK VS HEAP
// ============================================================

struct SmallObject {
  int x;
  int y;

  SmallObject(int x, int y) : x(x), y(y) {
    std::cout << "[Ctor] SmallObject\n";
  }

  ~SmallObject() { std::cout << "[Dtor] SmallObject\n"; }
};

void example_01_stack_vs_heap() {
  std::cout << "\n=== 1. Stack vs Heap ===\n";

  // Local object:
  // lifetime is tied to the scope.
  SmallObject stack_object(1, 2);

  // Dynamically allocated object:
  // lifetime continues until delete.
  SmallObject* heap_object = new SmallObject(3, 4);

  std::cout << "stack_object address = " << &stack_object << "\n";

  std::cout << "heap_object address = " << heap_object << "\n";

  delete heap_object;

  // Important:
  // "stack vs heap" is about storage duration / allocation strategy,
  // NOT about whether the object is automatically destroyed.
}

// ============================================================
// 2. new / delete
// ============================================================

class Widget {
 public:
  Widget(int value) : value(value) {
    std::cout << "[Ctor] Widget (" << value << ")\n";
  }

  ~Widget() { std::cout << "[Dtor] Widget (" << value << ")\n"; }

 private:
  int value;
};

void example_02_new_delete() {
  std::cout << "\n=== 2. new / delete ===\n";

  // new does two things:
  // 1. allocate raw memory
  // 2. construct the object
  Widget* p = new Widget(42);

  // delete does two things:
  // 1. call destructor
  // 2. release memory
  delete p;

  // Arrays require new[]/delete[].
  Widget* array = new Widget[3]{Widget(1), Widget(2), Widget(3)};

  delete[] array;

  // Matching rules:
  //
  // new       -> delete
  // new[]     -> delete[]
}

// ============================================================
// 3. Memory Leak
// ============================================================

void create_leak() {
  int* p = new int(42);

  // BUG:
  // p is lost without delete.
  //
  // The allocated memory can no longer be reached.
  //
  // This is a memory leak.
  (void)p;
}

void example_03_memory_leak() {
  std::cout << "\n=== 3. Memory Leak ===\n";

  create_leak();

  std::cout << "create_leak() intentionally leaked memory\n";

  // In a real program, use sanitizers:
  //
  // -fsanitize=address
  //
  // to detect many leaks and memory errors.
}

// ============================================================
// 4. RAII
// ============================================================

class File {
 public:
  explicit File(const char* path) : file(std::fopen(path, "w")) {
    if (!file) {
      throw std::runtime_error("failed to open file");
    }
    std::cout << "File opened\n";
  }

  ~File() {
    if (file) {
      std::fclose(file);
      std::cout << "File closed\n";
    }
  }

  void write(const char* text) { std::fprintf(file, "%s\n", text); }

  // A FILE* represents a unique resource.
  // Copying would be dangerous because both obejcts
  // might try to fclose() // the same FILE*.
  File(const File&) = delete;
  File& operator=(const File&) = delete;

 private:
  std::FILE* file;
};

void example_04_raii() {
  std::cout << "\n=== 4. RAII ===\n";

  try {
    File file("test.txt");

    file.write("Hello RAII");

    // Destructor still runs when exception is throw.
    throw std::runtime_error("something went wrong");

  } catch (const std::exception& e) {
    std::cout << "Caught exception: " << e.what() << "\n";
  }

  /*
    The important idea:

    {
        File file;
        ...
    }

    becomes conceptually:

        construct resource

        ...

        destroy resource

    even when leaving the scope because of an exception.
  */
}

// ============================================================
// 5. SMART POINTER OVERVIEW
// ============================================================

void example_05_smart_pointer_overview() {
  std::cout << "\n=== 5. Smart Pointer Overview ===\n";

  /*
    unique_ptr
    ----------
    Exactly one owner.

        unique_ptr ---> object


    shared_ptr
    ----------
    Multiple owners.

        shared_ptr --+
                     |
        shared_ptr --+--> object


    weak_ptr
    --------
    Non-owning reference.

        weak_ptr -------> object

    It does NOT keep the object alive.
  */

  auto unique = std::make_unique<int>(10);

  auto shared1 = std::make_shared<int>(20);
  auto shared2 = shared1;

  std::weak_ptr<int> weak = shared1;

  std::cout << "shared use_count = " << shared1.use_count() << "\n";

  std::cout << "weak use_count = " << weak.use_count() << "\n";

  (void)unique;
  (void)shared2;
}

// ============================================================
// 6. std::unique_ptr
// ============================================================

struct Resource {
  Resource(int id) : id(id) { std::cout << "[Ctor] Resource " << id << "\n"; }

  ~Resource() { std::cout << "[Dtor] Resource " << id << "\n"; }

  int id;
};

void consume_unique(std::unique_ptr<Resource> resource) {
  std::cout << "consume_unique owns Resource " << resource->id << "\n";
}

void example_06_unique_ptr() {
  std::cout << "\n=== 6. std::unique_ptr ===\n";

  // Preferred way.
  auto resource = std::make_unique<Resource>(1);

  // Ownership transfer.
  auto another = std::move(resource);

  assert(resource == nullptr);
  assert(another != nullptr);

  // unique_ptr cannot be copied.
  //
  // auto copy = another; // ERROR

  // unique_ptr can be moved.
  consume_unique(std::move(another));

  assert(another == nullptr);

  // unique_ptr makes ownership explicit in APIs.
  auto r = std::make_unique<Resource>(2);
  consume_unique(std::move(r));
}

// ============================================================
// 7. std::shared_ptr
// ============================================================

void example_07_shared_ptr() {
  std::cout << "\n=== 7. std::shared_ptr ===\n";

  auto a = std::make_shared<Resource>(10);

  std::cout << "count = " << a.use_count() << "\n";

  {
    auto b = a;

    std::cout << "count = " << a.use_count() << "\n";

    {
      auto c = b;

      std::cout << "count = " << a.use_count() << "\n";
    }

    std::cout << "after c dies, count = " << a.use_count() << "\n";
  }

  std::cout << "after b dies, count = " << a.use_count() << "\n";

  // Object is destoryed when the final shared_ptr disappears.
}

// ============================================================
// 8. std::weak_ptr
// ============================================================

struct Node {
  explicit Node(std::string name) : name(std::move(name)) {
    std::cout << "[Ctor] Node " << this->name << "\n";
  }

  ~Node() { std::cout << "[Dtor] Node " << name << "\n"; }

  std::string name;

  std::shared_ptr<Node> next;
};

void example_08_weak_ptr() {
  std::cout << "\n=== 8. std::weak_ptr ===\n";

  auto node = std::make_shared<Node>("A");

  std::weak_ptr<Node> observer = node;

  std::cout << "strong count = " << node.use_count() << "\n";

  std::cout << "weak count = " << observer.use_count() << "\n";

  // weak_ptr does NOT own the object.
  // We must call lock() to temporarily obtain ownership.

  if (auto locked = observer.lock()) {
    std::cout << "Object is alive: " << locked->name << "\n";
  }

  node.reset();

  // Object is now destroyed.
  // observer still exists, but it doesn't keep Node alive.
  if (observer.expired()) {
    std::cout << "Object is already destroyed\n";
  }

  if (auto locked = observer.lock()) {
    // won't happen
    std::cout << locked->name << "\n";
  } else {
    std::cout << "lock() failed\n";
  }
}

// ============================================================
// 9. shared_ptr CYCLE
// ============================================================

struct CycleNode {
  std::string name;

  explicit CycleNode(std::string name) : name(std::move(name)) {
    std::cout << "[Ctor] CycleNode " << this->name << "\n";
  }

  ~CycleNode() { std::cout << "[Dtor] CycleNode " << name << "\n"; }

  std::shared_ptr<CycleNode> next;
};

void example_09_shared_ptr_cycle() {
  std::cout << "\n=== 9. shared_ptr Cycle ===\n";

  auto a = std::make_shared<CycleNode>("A");
  auto b = std::make_shared<CycleNode>("B");

  std::weak_ptr<CycleNode> wa = a;
  std::weak_ptr<CycleNode> wb = b;

  a->next = b;
  b->next = a;

  std::cout << "a count = " << a.use_count() << "\n";
  std::cout << "b count = " << b.use_count() << "\n";

  a.reset();
  b.reset();

  std::cout << "CycleNode object not destroyed yet\n";

  /*
    No destructors run.

    Why?

    A -> B
    B -> A

    Even though local a and b are gone,
    the objects still own each other.

    Reference counts never reach zero.
  */

  if (auto locked = wa.lock()) {
    locked->next.reset();
  }

  if (auto locked = wb.lock()) {
    locked->next.reset();
  }
}

struct NonOwningNode {
  std::string name;

  std::shared_ptr<NonOwningNode> next;
  std::weak_ptr<NonOwningNode> prev;
};

void example_10_break_cycle_with_weak_ptr() {
  std::cout << "\n=== 10. Break Cycle with weak_ptr ===\n";

  auto a = std::make_shared<NonOwningNode>();
  auto b = std::make_shared<NonOwningNode>();

  a->next = b;

  // prev does not own a.
  b->prev = a;

  std::cout << "a count = " << a.use_count() << "\n";

  std::cout << "b count = " << b.use_count() << "\n";

  // Both objects are correctly destroyed.
}

// ============================================================
// 11. CUSTOM DELETERS
// ============================================================

// A deleter is used mainly with smart pointers, which decides what happens when
// pointer dies:
//   std::unique_ptr
//   std::shared_ptr
//
// Default deleter: `delete ptr;`
//
// But sometimes resource is NOT freed by `delete`. Examples:
//   fclose(FILE*)
//   free(malloc_ptr)
//   close(fd)
//   sqlite3_close(db)
//   custom pool return

// void deleter(T* ptr) -> int (*)(FILE*)
using FilePtr = std::unique_ptr<std::FILE, decltype(&std::fclose)>;

FilePtr open_file(const char* path) {
  std::FILE* file = std::fopen(path, "w");

  if (!file) {
    throw std::runtime_error("failed to open file");
  }

  return FilePtr(file, &std::fclose);
}

void example_11_custom_deleter() {
  std::cout << "\n=== 11. Custom Deleter ===\n";

  {
    auto file = open_file("test.txt");

    std::fprintf(file.get(), "Hello custom deleter\n");

    // When file goes out of scope:
    //
    // fclose(file.get()) is automatically called.
  }

  /*
    unique_ptr normally does:

        delete ptr;

    But FILE* requires:

        fclose(ptr);

    Therefore:

        unique_ptr<T, Deleter>

    lets us customize cleanup.

    Another common example:

    malloc() -> free()

    rather than:

    malloc() -> delete
  */
}

// ============================================================
// 12. ALLOCATORS - BASIC IDEA
// ============================================================

template <typename T>
class SimpleAllocator {
 public:
  using value_type = T;

  T* allocate(std::size_t n) {
    if (n > static_cast<std::size_t>(-1) / sizeof(T)) {
      throw std::bad_alloc();
    }

    std::cout << "Allocating " << n * sizeof(T) << " bytes\n";

    void* memory = ::operator new(n * sizeof(T));

    return static_cast<T*>(memory);
  }

  void deallocate(T* ptr, std::size_t n) noexcept {
    std::cout << "Deallocating " << n * sizeof(T) << " bytes\n";

    ::operator delete(ptr);
  }
};

void example_12_allocator() {
  std::cout << "\n=== 12. Allocator ===\n";

  SimpleAllocator<int> allocator;

  int* memory = allocator.allocate(3);

  /*
    Important:

    allocate()
    does NOT construct int objects.

    It only gives us raw memory.
  */

  memory[0] = 10;
  memory[1] = 20;
  memory[2] = 30;

  std::cout << memory[0] << "\n";
  std::cout << memory[1] << "\n";
  std::cout << memory[2] << "\n";

  allocator.deallocate(memory, 3);
}

// ============================================================
// 13. PLACEMENT new
// ============================================================

class ExpensiveObject {
 public:
  explicit ExpensiveObject(int value) : value(value) {
    std::cout << "[Ctor] ExpensiveObject " << this->value << "\n";
  }

  ~ExpensiveObject() {
    std::cout << "[Dtor] ExpensiveObject " << value << "\n";
  }

  void print() const { std::cout << "value = " << value << "\n"; }

 private:
  int value;
};

void example_13_placement_new() {
  std::cout << "\n=== 13. Placement new ===\n";

  // Step 1: allocate new memory
  void* memory = ::operator new(sizeof(ExpensiveObject));

  // Step 2: construct object inside the memory.
  ExpensiveObject* object = new (memory) ExpensiveObject(42);

  // Step 3: explicitly destroy object.
  object->~ExpensiveObject();

  // Step 4: release raw memory.
  ::operator delete(memory);

  /*
    Normal new:

        new T(...)
            |
            +-- allocate memory
            +-- construct object


    Placement new:

        operator new(...)
            |
            +-- allocate memory

        placement new
            |
            +-- construct object
  */
}

// ============================================================
// 14. ALIGNMENT
// ============================================================

struct Normal {
  char a;
  int b;
  double c;
};

struct Aligned {
  alignas(64) int value;
};

void example_14_alignment() {
  std::cout << "\n=== 14. Alignment ===\n";

  std::cout << "alignof(char) = " << alignof(char) << "\n";

  std::cout << "alignof(int) = " << alignof(int) << "\n";

  std::cout << "alignof(double) " << alignof(double) << "\n";

  std::cout << "alignof(Normal) " << alignof(Normal) << "\n";

  std::cout << "sizeof(Normal) " << sizeof(Normal) << "\n";

  std::cout << "alignof(Aligned) " << alignof(Aligned) << "\n";

  Aligned object;

  std::uintptr_t address = reinterpret_cast<std::uintptr_t>(&object);

  std::cout << "address % 64 = " << address % 64 << '\n';
  /*
    alignas(64) means:

        the object must start at an address
        divisible by 64.

    This is useful for things such as:

    - SIMD
    - cache-line alignment
    - avoiding false sharing
    - hardware interfaces
  */
}

// ============================================================
// 15. OBJECT MEMORY LAYOUT
// ============================================================

struct Empty {};

struct NonVirtual {
  void foo();
};

struct Virtual {
  virtual void foo();
};

struct Plain {
  char a;
  int b;
  double c;
};

class Base {
 public:
  virtual ~Base() = default;

  int base_value = 1;
};

class Derived : public Base {
 public:
  int derived_value = 2;
};

void example_15_object_layout() {
  std::cout << "\n=== 15. Object Memory Layout ===\n";

  std::cout << "sizeof(Empty) = " << sizeof(Empty) << "\n";
  std::cout << "sizeof(NonVirtual) = " << sizeof(NonVirtual) << "\n";
  std::cout << "sizeof(Virtual) = " << sizeof(Virtual) << "\n";

  Plain object;

  std::cout << "sizeof(Plain) = " << sizeof(Plain) << "\n";

  std::cout << "&object = " << static_cast<void*>(&object) << "\n";

  std::cout << "&object.a = " << static_cast<void*>(&object.a) << "\n";

  std::cout << "&object.b = " << static_cast<void*>(&object.b) << "\n";

  std::cout << "&object.c = " << static_cast<void*>(&object.c) << "\n";

  Base base;
  Derived derived;

  std::cout << "sizeof(Base) = " << sizeof(Base) << '\n';

  std::cout << "sizeof(Derived) = " << sizeof(Derived) << '\n';

  std::cout << "Base alignment = " << alignof(Base) << '\n';

  std::cout << "Derived alignment = " << alignof(Derived) << '\n';

  std::cout << "&base        = " << &base << '\n';

  std::cout << "&derived     = " << &derived << '\n';

  std::cout << "&base_value  = " << &base.base_value << '\n';

  std::cout << "&derived_base_value = " << &derived.base_value << '\n';

  std::cout << "&derived_value = " << &derived.derived_value << '\n';
  /*
    Plain may conceptually look like:

        +----------------+
        | char a         |
        +----------------+
        | padding        |
        +----------------+
        | int b          |
        +----------------+
        | double c       |
        +----------------+

    Padding exists because of alignment.

    For a polymorphic object:

      Base
      |
      0
      ┌─────────────────────┐
      │ vptr                │  8
      ├─────────────────────┤
      │ base_value          │  4
      ├─────────────────────┤
      │ padding             │  4
      └─────────────────────┘
                          16

        Derived
        |
        0
        ┌─────────────────────┐
        │ vptr                │  8
        ├─────────────────────┤
        │ base_value          │  4
        ├─────────────────────┤
        │ derived_value       │  4
        └─────────────────────┘
                            16
  */
}

// ============================================================
// 16. DANGLING POINTER
// ============================================================

const int* bad_pointer() {
  int local = 42;

  // Returning the address of a local variable
  // is dangerous.
  return &local;
}

void example_16_dangling_pointer() {
  std::cout << "\n=== 16. Dangling Pointer ===\n";

  const int* p = bad_pointer();

  /*
      bad_pointer() returned.

      local no longer exists.

      Therefore p points to memory where
      the object lifetime has ended.

      p is a dangling pointer.

      NEVER dereference p.
  */

  std::cout << "p is dangling and must not be dereferenced\n";

  // Another very common example:

  std::vector<int> values = {1, 2, 3};

  int* element = &values[0];

  std::cout << "Before reallocation: " << *element << "\n";

  values.reserve(100);

  /*
       reserve() may move the vector's elements
       to a different memory allocation.

       element may now be dangling.
   */

  (void)element;
}

// ============================================================
// 17. USE-AFTER-FREE
// ============================================================

void example_17_use_after_free() {
  std::cout << "\n=== 17. Use-after-free ===\n";

  int* p = new int(42);

  delete p;

  /*
      p still contains an address.

      But the object at that address
      no longer exists.

      Therefore:

          *p

      is a use-after-free.

      DO NOT DO THIS.
  */

  // std::cout << *p;    // BUG

  // Better:

  p = nullptr;

  /*
      A null pointer clearly communicates:

          "I no longer point to an object."
  */
}

// ============================================================
// 18. DOUBLE FREE
// ============================================================

void example_18_double_free() {
  std::cout << "\n=== 18. Double Free ===\n";

  int* p = new int(42);

  delete p;

  /*
      BUG:

          delete p;

      again would attempt to release the same
      allocation twice.

      This is undefined behavior.
  */

  // delete p;   // BUG

  // Setting the pointer to nullptr prevents this
  // particular mistake:

  p = nullptr;

  delete p;  // safe: deleting nullptr does nothing
}

// ============================================================
// 19. WHY RAII SOLVES MANY OF THESE PROBLEMS
// ============================================================

void example_19_raii_vs_raw_pointer() {
  std::cout << "\n=== 19. RAII vs Raw Pointer ===\n";

  // BAD:

  /*
      Resource* resource = new Resource(1);

      if (something_goes_wrong()) {
          return;        // LEAK
      }

      delete resource;
  */

  // GOOD:

  auto resource = std::make_unique<Resource>(1);

  /*
      No explicit delete.

      No leak when returning early.

      No leak when exception is thrown.

      Ownership is explicit.
  */

  (void)resource;
}

// ============================================================
// 20. FINAL: SMALL POOL ALLOCATOR
// ============================================================

class ObjectPool {
 public:
  explicit ObjectPool(std::size_t capacity) : capacity(capacity) {
    memory = ::operator new(capacity * sizeof(Resource));
  }

  ~ObjectPool() { ::operator delete(memory); }

  Resource* create(int id) {
    if (size >= capacity) {
      throw std::bad_alloc();
    }

    char* start = static_cast<char*>(memory);

    void* address = start + size * sizeof(Resource);

    ++size;

    return new (address) Resource(id);
  }

  void destroy(Resource* object) {
    if (!object) return;

    object->~Resource();
  }

 private:
  void* memory;

  std::size_t capacity = 0;
  std::size_t size = 0;
};

void example_20_object_pool() {
  std::cout << "\n=== 20. Object Pool ===\n";

  ObjectPool pool(3);

  Resource* a = pool.create(1);
  Resource* b = pool.create(2);

  std::cout << "a = " << a << '\n';
  std::cout << "b = " << b << '\n';

  pool.destroy(a);
  pool.destroy(b);

  /*
      This combines several concepts:

      raw allocation
          |
          v
      alignment
          |
          v
      placement new
          |
          v
      object lifetime
          |
          v
      explicit destructor
          |
          v
      raw memory release
  */
}

//////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////

int run() {
  example_01_stack_vs_heap();

  example_02_new_delete();

  // Intentionally leaks memory.
  example_03_memory_leak();

  example_04_raii();

  example_05_smart_pointer_overview();
  example_06_unique_ptr();
  example_07_shared_ptr();
  example_08_weak_ptr();

  // Intentionally demonstrates a shared_ptr cycle.
  example_09_shared_ptr_cycle();

  example_10_break_cycle_with_weak_ptr();

  example_11_custom_deleter();

  example_12_allocator();
  example_13_placement_new();

  example_14_alignment();
  example_15_object_layout();

  // Demonstrates a dangling pointer, but does not dereference it.
  example_16_dangling_pointer();

  example_17_use_after_free();
  example_18_double_free();

  example_19_raii_vs_raw_pointer();

  example_20_object_pool();

  return 0;
}

}  // namespace MM
