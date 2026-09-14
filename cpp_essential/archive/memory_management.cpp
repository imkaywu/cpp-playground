#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <vector>

#include "benchmark.cpp"

namespace MM {

// -----------
// new[] / delete[]
// -----------
class Base {
 public:
  Base() { std::cout << "[Ctor] Base\n"; }
  ~Base() { std::cout << "[Dtor] Base\n"; }
  int n;
};

void test_new_delete_internal() {
  // Base* p = new Base[3];
  // delete[] p;

  // Solution 1: over allocation
  size_t n = 3;
  const size_t WORDSIZE = 8;  // for 64 bit system

  char* head = (char*)operator new(WORDSIZE + n * sizeof(Base));
  Base* p = (Base*)(head + WORDSIZE);
  *(size_t*)head = n;
  for (auto i = 0; i < n; ++i) {
    new (p + i) Base();
  }

  size_t n2 = *(size_t*)((char*)p - WORDSIZE);
  for (auto i = 0; i < n2; ++i) {
    (p + i)->~Base();
  }
  operator delete(head);

  // Solution 2: associative array: watch p2z7SJ5MWV8
}

// -----------
// RAII
// -----------
class FileRAII {
  FILE* file;

 public:
  FileRAII(const char* name, const char* mode) {
    file = fopen(name, mode);
    if (!file) throw std::runtime_error("Failed to open file");
    std::cout << "File opended\n";
  }

  ~FileRAII() {
    if (file) {
      fclose(file);
      std::cout << "File closed\n";
    }
  }

  void write(const char* msg) { fprintf(file, "%s\n", msg); }

  // disable copying (if resource is unique)
  FileRAII(const FileRAII&) = delete;
  FileRAII& operator=(const FileRAII&) = delete;
};

void test_RAII() {
  try {
    FileRAII file("out.txt", "w");
    file.write("Hello RAII");

    // exception safety: destructor always runs, even if an exception is thrown
    throw std::runtime_error("Simulated exception");

  } catch (...) {
    std::cout << "Exception caught\n";
  }
}

// -----------
// Smart pointer
// -----------
struct Node {
  std::string name;
  std::shared_ptr<Node> next;

  // Use pass-by-value: 1 copy + 1 move
  //
  // Avoid duplicationg:
  //   Node(const std::string& n)
  //   Node(std::string&& n)
  Node(std::string n) : name(std::move(n)) {
    std::cout << "[Ctor] node: " << name << "\n";
  }

  ~Node() { std::cout << "[Dtor] node: " << name << "\n"; }
};

void test_smart_pointer() {
  std::cout << "--- Create two nodes ---\n";
  std::shared_ptr<Node> a =
      std::shared_ptr<Node>(new Node("A"));  // a.use_count == 1
  auto b = std::make_shared<Node>("B");      // b.use_count == 1

  std::weak_ptr<Node> wa = a;
  std::weak_ptr<Node> wb = b;

  std::cout << "Initial counts: a=" << a.use_count() << ", b=" << b.use_count()
            << "\n";

  std::cout << "--- Link A -> B ---\n";
  a->next = b;
  std::cout << "After A -> B, a=" << a.use_count() << ", b=" << b.use_count()
            << "\n";

  std::cout << "--- Link B -> A (create cycle) ---\n";
  b->next = a;
  std::cout << "After B -> A, a=" << a.use_count() << ", b=" << b.use_count()
            << "\n";

  std::cout << "--- Reset local shared_ptrs a & b ---\n";
  // NOTE: reset() sets the shared pointer to null, and decrease strong/shared
  // count by 1.
  a.reset();
  b.reset();
  if (a == nullptr) {
    std::cout << "After reset locals, a is nullptr, a.use_count="
              << a.use_count() << "\n";
  }
  std::cout << "After reset locals, wa.use_count=" << wa.use_count()
            << ", wb.use_count=" << wb.use_count() << "\n";

  std::cout << "--- Break the cycle by resetting internal 'next' links ---\n";
  // NOTE: lock atomically tries to convert a weak reference into a
  // shared_ptr.
  if (auto sa = wa.lock()) {
    sa->next.reset();  // release shared_ptr to B
  }
  if (auto sb = wb.lock()) {
    sb->next.reset();  /// release shared_ptr to A
  }
  std::cout << "After breaking internal links, wa.use_count=" << wa.use_count()
            << ", wb.use_count=" << wb.use_count() << "\n";
}

// -----------
// Allocator
// -----------
template <typename T>
class CAllocator {
 public:
  using value_type = T;

  CAllocator() = default;

  template <typename U>
  CAllocator(const CAllocator<U>&) noexcept {}

  ~CAllocator() = default;

  T* allocate(size_t n) {
    if (n > max_size()) {
      throw std::bad_alloc();
    }

    void* ptr = std::malloc(n * sizeof(T));

    if (ptr == nullptr) {
      throw std::bad_alloc();
    }

    return static_cast<T*>(ptr);
  }

  void deallocate(T* ptr, size_t) noexcept { free(ptr); }

  constexpr size_t max_size() const noexcept {
    return static_cast<std::size_t>(-1) / sizeof(T);
  }

  template <typename U>
  struct rebind {
    using other = CAllocator<U>;
  };

  template <typename U>
  constexpr bool operator==(const CAllocator<U>&) const noexcept {
    return true;
  }

  template <typename U>
  constexpr bool operator!=(const CAllocator<U>&) const noexcept {
    return false;
  }
};

template <typename T>
class LinearAllocator {
 public:
  using value_type = T;

 public:
  explicit LinearAllocator(size_t size)
      : block(std::make_shared<Block>(size)) {}

  LinearAllocator(const LinearAllocator&) = default;

  // NOTE: template constructor:
  //   Containers internally rebind allocators to other types
  //
  //   std::list<int>
  //
  //   internally allocates `_List_node<int>`
  //
  //   STL needs `LinearAllocator<_List_node<int>>` constructed from
  //   `LinearAllocator<int>>`
  template <typename U>
  LinearAllocator(const LinearAllocator<U>& other) noexcept
      : block(other.block) {}

  ~LinearAllocator() = default;

  // NOTE: compiler attribute, warning if caller ignores return value.
  [[nodiscard]]
  T* allocate(size_t n) {
    size_t bytes = n * sizeof(T);
    size_t alignment = alignof(T);

    uintptr_t addr = reinterpret_cast<uintptr_t>(block->current);

    size_t padding = (alignment - (addr % alignment)) % alignment;

    if (block->used + padding + bytes > block->size) {
      throw std::bad_alloc();
    }

    addr += padding;

    T* result = reinterpret_cast<T*>(addr);

    block->current = reinterpret_cast<char*>(addr + bytes);

    block->used += padding + bytes;

    return result;
  }

  void deallocate(T*, size_t) noexcept {
    // NOTE: Linear allocator does not support individual deallocation.
    // no-op
  }

  void reset() {
    block->current = block->start;
    block->used = 0;
  }

  // NOTE: Given allocator for T, create equivalent allocator for U
  //   LinearAllocator<int> -> LinearAllocator<ListNode<int>>
  template <typename U>
  struct rebind {
    using other = LinearAllocator<U>;
  };

  // NOTE: Containers need to know "can I transfer memory ownership between
  // these containers safely?"
  //
  // allocators with the same memory block are interchangeable
  bool operator==(const LinearAllocator& other) const noexcept {
    return block == other.block;
  }

  bool operator!=(const LinearAllocator& other) const noexcept {
    return !(*this == other);
  }

 private:
  struct BlockDeleter {
    void operator()(void* ptr) const noexcept { ::operator delete(ptr); }
  };

  struct Block {
    std::unique_ptr<void, BlockDeleter> memory;

    char* start = nullptr;
    char* current = nullptr;

    size_t size = 0;
    size_t used = 0;

    explicit Block(size_t sz)
        : memory(::operator new(sz)),
          start(static_cast<char*>(memory.get())),
          current(start),
          size(sz) {}
  };

  std::shared_ptr<Block> block;

  // NOTE: grants friendship between ALL specializations:
  //    LinearAllocator<int>
  //    LinearAllocator<double>
  // because the rebinding constructors need access to |block|.
  template <typename>
  friend class LinearAllocator;
};

void test_allocator() {
  { benchmark_vector_push("CAllocator", CAllocator<int>{}, 1'000'000); }
  {
    benchmark_vector_push(
        "LinearAllocator", LinearAllocator<int>(1024 * 1024 * 64), 1'000'000);
  }
}

//////////////////////////////////////////////////////////////
// (47) Alignment - avoid false sharing for hot counters
//////////////////////////////////////////////////////////////

struct AlignedCounter {
  alignas(64) std::atomic<size_t> value{0};
};

//////////////////////////////////////////////////////////////
// (48) Memory layout - polymorphic log entry
//////////////////////////////////////////////////////////////

class LogEntry {
 public:
  virtual void write(FILE* f) const = 0;
  virtual ~LogEntry() = default;
};

// Derived adds extra fields -> impacts layout
class TextLog : public LogEntry {
 private:
  char* msg;

 public:
  TextLog(const char* m) {
    std::cout << "[Ctor] Log\n";
    size_t len = std::strlen(m) + 1;
    msg = new char[len];
    std::strncpy(msg, m, len);
  }

  ~TextLog() {
    std::cout << "[Dtor] Log\n";
    delete[] msg;
  }

  void write(FILE* f) const override { fprintf(f, "%s\n", msg); }
};

//////////////////////////////////////////////////////////////
// (45,46) Custom allocator (pool) + placement new
//////////////////////////////////////////////////////////////

class LogPool {
 private:
  std::vector<void*> free_list;

 public:
  LogPool(size_t capacity) {
    free_list.reserve(capacity);
    for (size_t i = 0; i < capacity; ++i) {
      free_list.push_back(::operator new(256));  // fixed block
    }
  }

  ~LogPool() {
    for (void* p : free_list) {
      ::operator delete(p);
    }
  }

  // T is of type |LogEntry|
  template <typename T, typename... Args>
  T* create(Args&&... args) {
    if (free_list.empty()) throw std::bad_alloc();

    void* mem = free_list.back();
    free_list.pop_back();

    // placement new
    return new (mem) T(std::forward<Args>(args)...);
  }

  void destroy(LogEntry* entry) {
    if (!entry) return;

    entry->~LogEntry();  // virtual dctor required
    free_list.push_back(entry);
  }
};

//////////////////////////////////////////////////////////////
// (39-44) RAII handle + custom deleter
//////////////////////////////////////////////////////////////

// NOTE: this class' only job is wrapping a pointer and calling a custom
// cleanup method in destructor, a smart pointer with custom deleter is a
// better solution.
/*
class LogHandler {
 private:
  LogEntry* entry;
  LogPool* pool;

 public:
  LogHandler(LogEntry* e, LogPool* p) : entry(e), pool(p) {}

  ~LogHandler() {
    if (entry) pool->destroy(entry);
  }

  LogEntry* operator->() { return entry; }

  // move only (unique ownership)
  LogHandler(const LogHandler&) = delete;
  LogHandler& operator=(const LogHandler&) = delete;

  LogHandler(LogHandler&& other) noexcept
      : entry(other.entry), pool(other.pool) {
    other.entry = nullptr;
  }

  LogHandler& operator=(LogHandler&& other) noexcept {
    if (this == &other) return *this;

    if (entry) pool->destroy(entry);

    entry = other.entry;
    pool = other.pool;
    other.entry = nullptr;

    return *this;
  }
};
*/

//////////////////////////////////////////////////////////////
// (2) Custom deleter - wrapper FILE*
//////////////////////////////////////////////////////////////

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

// `int fclose(FILE* stream);`
using FilePtr = std::unique_ptr<FILE, int (*)(FILE*)>;  // 2nd arg: function
                                                        // pointer
// Alternative:
// using FilePtr = std::unique_ptr<FILE, decltype(&fclose)>;

FilePtr open_file(const char* path) {
  return FilePtr(fopen(path, "w"), &fclose);
}

struct PoolDeleter {
  LogPool* pool;

  void operator()(LogEntry* log) { pool->destroy(log); }
};

using LogPtr = std::unique_ptr<LogEntry, PoolDeleter>;

//////////////////////////////////////////////////////////////
// Logger system
//////////////////////////////////////////////////////////////

class Logger {
 private:
  LogPool pool;
  // NOTE: Deprecated by |LogPtr|
  // std::vector<LogHandler> queue;
  std::vector<LogPtr> queue;

  AlignedCounter write_count;  // avoid false sharing

  FilePtr file;

 public:
  Logger(size_t pool_size, const char* path)
      : pool(pool_size), file(open_file(path)) {}

  // NOTE: T is of type |LogEntry|
  template <typename T, typename... Args>
  void log(Args&&... args) {
    // allocate log entry from pool
    auto* raw = pool.create<T>(std::forward<Args>(args)...);

    queue.emplace_back(raw, PoolDeleter{&pool});  // RAII ownership

    write_count.value.fetch_add(1, std::memory_order_relaxed);
  }

  void flush() {
    for (auto& h : queue) {
      h->write(file.get());
    }

    queue.clear();  // triggers destruction -> return to pool
  }

  size_t count() const { return write_count.value.load(); }
};

//////////////////////////////////////////////////////////////
// (1) Memory leak example (intentional bug)
//////////////////////////////////////////////////////////////

void leak_example() {
  TextLog* log = new TextLog("leak");  // never deleted
}

//////////////////////////////////////////////////////////////
// (49,50) Dangling + invalidation pitfalls
//////////////////////////////////////////////////////////////

void dangerous_patterns() {
  // dangling pointer
  const char* p;
  {
    char buf[32];
    std::strcpy(buf, "temp");
    p = buf;
  }
  // p now dangling

  // vector invalidation
  std::vector<int> v = {1, 2, 3};
  int* ptr = &v[0];
  v.push_back(4);  // may reallocate
  // ptr now invalid
}

//////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////

int run() {
  std::cout << "\n--- Logger System ---\n";

  Logger logger(4, "log.txt");

  logger.log<TextLog>("hello world");
  logger.log<TextLog>("another log");

  logger.flush();

  std::cout << "Log count: " << logger.count() << "\n";

  std::cout << "\n--- Memory Layout Insight ---\n";
  std::cout << "sizeof(LogEntry): " << sizeof(LogEntry) << "\n";
  std::cout << "sizeof(TextLog): " << sizeof(TextLog) << "\n";

  std::cout << "\n--- Alignment ---\n";
  std::cout << "alignof(AlignedCounter): " << alignof(AlignedCounter) << "\n";

  std::cout << "=== new[]/delte[] ===\n";
  test_new_delete_internal();

  std::cout << "=== RAII ===\n";
  test_RAII();

  std::cout << "=== Smart Pointers ===\n";
  test_smart_pointer();

  std::cout << "=== Allocator ===\n";
  test_allocator();

  std::cout << "\n--- End ---\n";

  return 0;
}

}  // namespace MM
