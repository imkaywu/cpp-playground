#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <vector>

namespace MM {

//////////////////////////////////////////////////////////////
// (47) Alignment - avoid false sharing for hot counters
//////////////////////////////////////////////////////////////

struct alignas(64) AlignedCounter {
  std::atomic<size_t> value{0};
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
    size_t len = std::strlen(m) + 1;
    msg = new char[len];
    std::strncpy(msg, m, len);
  }

  ~TextLog() { delete[] msg; }

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

    entry->~LogEntry();  // virturl dctor required
    free_list.push_back(entry);
  }
};

//////////////////////////////////////////////////////////////
// (39-44) RAII handle + custom deleter
//////////////////////////////////////////////////////////////

class LogHandle {
 private:
  LogEntry* entry;
  LogPool* pool;

 public:
  LogHandle(LogEntry* e, LogPool* p) : entry(e), pool(p) {}

  ~LogHandle() {
    if (entry) pool->destroy(entry);
  }

  LogEntry* operator->() { return entry; }

  // move only (unique ownership)
  LogHandle(const LogHandle&) = delete;
  LogHandle& operator=(const LogHandle&) = delete;

  LogHandle(LogHandle&& other) noexcept : entry(other.entry), pool(other.pool) {
    other.entry = nullptr;
  }

  LogHandle& operator=(LogHandle&& other) noexcept {
    if (this == &other) return *this;

    if (entry) pool->destroy(entry);

    entry = other.entry;
    pool = other.pool;
    other.entry = nullptr;

    return *this;
  }
};

//////////////////////////////////////////////////////////////
// (2) Custom deleter - wrapper FILE*
//////////////////////////////////////////////////////////////

using FilePtr = std::unique_ptr<FILE, decltype(&fclose)>;

FilePtr open_file(const char* path) {
  return FilePtr(fopen(path, "w"), &fclose);
}

//////////////////////////////////////////////////////////////
// Logger system
//////////////////////////////////////////////////////////////

class Logger {
 private:
  LogPool pool;
  std::vector<LogHandle> queue;

  AlignedCounter write_count;  // avoid false sharing

  FilePtr file;

 public:
  Logger(size_t pool_size, const char* path)
      : pool(pool_size), file(open_file(path)) {}

  template <typename T, typename... Args>
  void log(Args&&... args) {
    // allocate log entry from pool
    LogEntry* e = pool.create<T>(std::forward<Args>(args)...);

    queue.emplace_back(e, &pool);  // RAII ownership

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

  std::cout << "\n--- End ---\n";

  return 0;
}

}  // namespace MM
