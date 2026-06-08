#include <barrier>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>

namespace MT {

// ------------
// Threads
// ------------

void function() { std::cout << "function says: here is a message\n"; }

class Functor {
 public:
  Functor() = default;
  Functor(const Functor&) { std::cout << "[Copy ctor] functor\n"; }
  Functor(Functor&&) { std::cout << "[Move ctor] functor\n"; }

  void operator()(std::string& msg) {
    std::cout << "thread says: " << msg << "\n";
    msg = "here is another message";
  }
};

// Thread-local variable: each thread gets its own copy
thread_local int thread_counter = 0;

void work(int id, const std::string& name) {
  ++thread_counter;

  std::cout << "Thread " << id << " (" << name << ") running on ID "
            << std::this_thread::get_id()
            << ", thread_local counter = " << thread_counter << "\n";

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  std::cout << "Thread " << id << " done\n";
}

void increment_thread(int& x) {
  for (int i = 0; i < 5; ++i) {
    ++x;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

int test_thread() {
  std::cout << "--- Thread basics ---\n";
  std::thread t0(function);
  t0.join();

  std::string msg = "here is some message";
  Functor functor;
  std::thread t1(functor, std::ref(msg));  // t1 starts running

  // std::thread t2 = t1; // thread not copyable
  std::thread t2 = std::move(t1);  //

  try {
    // NOTE: in case this block throws an error and thread fails to join
    for (int i = 0; i < 5; ++i) {
      std::cout << "from main: " << i << "\n";
    }
  } catch (...) {
    t2.join();
    throw;
  }

  t2.join();

  std::cout << "main says: " << msg << "\n";

  std::cout << "\n--- Variable in Thread ---\n";
  std::cout << "Main thread ID: " << std::this_thread::get_id() << "\n";

  std::thread t3(work, 1, "Alpha");
  std::thread t4(work, 2, "Beta");

  int shared_value = 0;
  std::thread t5(increment_thread, std::ref(shared_value));

  std::cout << "\nBefore joining, joinable states:\n";
  std::cout << "t3.joinable(): " << t3.joinable() << "\n";
  std::cout << "t4.joinable(): " << t4.joinable() << "\n";
  std::cout << "t5.joinable(): " << t5.joinable() << "\n";

  t3.join();
  t4.join();
  t5.detach();

  std::cout << "\nAfter joining/detaching:\n";
  std::cout << "t3.joinable(): " << t3.joinable() << "\n";
  std::cout << "t4.joinable(): " << t4.joinable() << "\n";
  std::cout << "t5.joinable(): " << t5.joinable() << "\n";

  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  std::cout << "\nShared value after detached thread increment: "
            << shared_value << "\n";

  std::cout << "\nMain thread done.\n";

  return 0;
}

// ------------
// Mutex
// ------------
// +
// ------------
// Lock guards
// ------------

// Global shared state
int shared_data = 0;

std::mutex mtx;
// same thread can lock it multiple times (used in recursion)
std::recursive_mutex re_mtx;
// allows multiple readers, but a single writer to hold it
std::shared_mutex rw_mtx;

// 1. mutex + lock_guard
// lock_guard: RAII wrapper for mutex, auto unlock at scope exit
void increment_with_mutex() {
  std::lock_guard<std::mutex> lock(mtx);
  shared_data++;
  std::cout << "[increment_with_mutex] shared data: " << shared_data << "\n";
}

// 2. recursive_mutex
void recursive_func(int n) {
  if (n <= 0) return;
  re_mtx.lock();
  std::cout << "re_mtx depth: " << n << "\n";
  recursive_func(n - 1);
  re_mtx.unlock();
}

// 3. shared_mutex
void reader(int id) {
  std::shared_lock<std::shared_mutex> lock(rw_mtx);
  std::cout << "Reader " << id << " reads shared_data = " << shared_data
            << "\n";
}

void writer(int id) {
  std::unique_lock<std::shared_mutex> lock(rw_mtx);
  shared_data++;
  std::cout << "Writer " << id << " updates shared_data = " << shared_data
            << "\n";
}

// 4. unique_lock + defer/adopt/try_lock
// unique_lock: more flexible lock
void unique_lock_example() {
  std::unique_lock<std::mutex> lock(mtx, std::defer_lock);  // not locked yet
  // try_lock: non-blocking
  if (lock.try_lock()) {
    std::cout << "unique_lock acquired using try_lock()\n";
  }
  lock.unlock();

  mtx.lock();
  std::unique_lock<std::mutex> adopted_lock(mtx, std::adopt_lock);
  std::cout << "unique_lock adopted existing lock\n";
  // lock auto-unlocked when goes out of scope
}

void test_mutex_and_lock_guard() {
  std::cout << "--- 1. Basic mutex and lock_guard ---\n";
  std::thread t1(increment_with_mutex);
  std::thread t2(increment_with_mutex);
  t1.join();
  t2.join();

  std::cout << "--- 2. recursive_mutex ---\n";
  recursive_func(3);

  std::cout << "--- 3. shared_mutex (readers/writers) ---\n";
  std::vector<std::thread> readers, writers;
  for (int i = 0; i < 3; ++i) readers.emplace_back(reader, i);
  for (int i = 0; i < 2; ++i) writers.emplace_back(writer, i);
  for (auto& t : readers) t.join();
  for (auto& t : writers) t.join();

  std::cout << "--- 4. unique_lock ---\n";
  unique_lock_example();
}

// ------------
// Condition Variable
// Design Pattern: Producer-Consumer Pattern
// ------------

std::condition_variable cv;
bool ready = false;

// condition_variable (wait/notify)
void notifier() {
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  {
    std::lock_guard<std::mutex> lock(mtx);
    ready = true;
    std::cout << "Notifier: setting ready = true\n";
  }
  cv.notify_one();  // wake up one waiting thread
}

void worker() {
  std::unique_lock<std::mutex> lock(mtx);
  std::cout << "Worker waiting...\n";
  // cv.wait(lock, predicate): put thread to sleep until predicate is true
  cv.wait(lock, [] { return ready; });  // waits until ready==true
  std::cout << "Worker proceeds\n";
}

std::deque<int> q;
std::condition_variable cond;

void produce() {
  int count = 5;
  while (count > 0) {
    std::unique_lock<std::mutex> locker(mtx);
    q.push_front(count);
    locker.unlock();
    cond.notify_one();
    count--;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void consume() {
  int data = 0;
  while (data != 1) {
    std::unique_lock<std::mutex> locker(mtx);
    cond.wait(locker, []() { return !q.empty(); });
    data = q.back();
    q.pop_back();
    locker.unlock();
    std::cout << "[consumer] queue value: " << data << "\n";
  }
}

const int BUFFER_SIZE = 8;
const int DATA_SIZE = 320;  // 40 cycles
struct SafeQueue {
  char buffer[BUFFER_SIZE];
  size_t head;
  size_t tail;
  std::mutex mtx;
  std::condition_variable_any cv;
};

void write(SafeQueue& queue) {
  for (int i = 0; i < DATA_SIZE; ++i) {
    std::unique_lock<std::mutex> locker(queue.mtx);

    // check if queue is full: (tail+1)%BUFFER_SIZE==head
    queue.cv.wait(locker, [&queue]() {
      return !((queue.tail + 1) % BUFFER_SIZE == queue.head);
    });

    queue.buffer[queue.tail] = 'a' + i % 26;
    queue.tail = (queue.tail + 1) % BUFFER_SIZE;

    locker.unlock();
    queue.cv.notify_one();
  }
}

void read(SafeQueue& queue) {
  for (int i = 0; i < DATA_SIZE; ++i) {
    std::unique_lock<std::mutex> locker(queue.mtx);

    // check if queue is empty: head==tail
    queue.cv.wait(locker, [&queue]() { return !(queue.head == queue.tail); });

    std::cout << "#" << i << ": " << queue.buffer[queue.head] << ", ";

    queue.head = (queue.head + 1) % BUFFER_SIZE;

    locker.unlock();
    queue.cv.notify_one();
  }

  std::cout << "\n";
}

void test_condition_variable() {
  std::thread signaler(notifier);
  std::thread waiter(worker);
  signaler.join();
  waiter.join();

  std::thread producer(produce);
  std::thread consumer(consume);
  producer.join();
  consumer.join();

  SafeQueue safe_queue{};
  std::thread writer(write, std::ref(safe_queue));
  std::thread reader(read, std::ref(safe_queue));
  writer.join();
  reader.join();
}

// ------------
// Deadlock
// ------------
// How to avoid:
// 1. prefer locking a single mutex at a time
// 2. Avoid locking a mutex and calling a user-defined method
// 3. Lock mutexes in the same order
// 4. Use std::lock to lock multiple mutexes

std::mutex m1, m2, m3, m4;

void t1() {
  std::lock_guard<std::mutex> l1(m1);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::lock_guard<std::mutex> l2(m2);

  std::cout << "t1 done\n";
}

void t2() {
  std::lock_guard<std::mutex> l1(m2);
  std::lock_guard<std::mutex> l2(m1);

  std::cout << "t2 done\n";
}

void safe_t1() {
  std::lock(m3, m4);
  std::lock_guard<std::mutex> l1(m3, std::adopt_lock);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::lock_guard<std::mutex> l2(m4, std::adopt_lock);

  std::cout << "safe t1 done\n";
}

void safe_t2() {
  std::lock(m3, m4);
  std::lock_guard<std::mutex> l1(m3, std::adopt_lock);
  std::lock_guard<std::mutex> l2(m4, std::adopt_lock);

  std::cout << "safe t2 done\n";
}

void test_deadlock() {
  // std::thread a(t1);
  // std::thread b(t2);
  std::thread a2(safe_t1);
  std::thread b2(safe_t2);

  a2.join();
  b2.join();
  // a.join();
  // b.join();  // deadlock risk
}

// ------------
// Atomic
// ------------
// +
// ------------
// Memory Order Model
// ------------
// https://www.ramtintjb.com/blog/memory-ordering

std::atomic<int> counter{0};

void increment() {
  for (int i = 0; i < 1000; ++i) {
    counter.fetch_add(1, std::memory_order_relaxed);
  }
}

/*
struct Data { int value; };
Data shared{0};
*/

std::atomic_flag atomic_flag_lock =
    ATOMIC_FLAG_INIT;  // Always initializes to false

void spinlock() {
  // Atomically sets the flag to true and returns its previous value.
  // - If the lock is acquired: by another thread, keeps spinning
  // - If the lock is released: acquire the lock, set to true, return false
  while (atomic_flag_lock.test_and_set(std::memory_order_acquire)) {
    // busy-wait (spin)
  }

  std::cout << "Atomic flag lock acquired by thread"
            << std::this_thread::get_id() << "\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Atomically sets the flag to false.
  atomic_flag_lock.clear(std::memory_order_release);
}

std::atomic<int> d{0};
std::atomic<bool> ready2{false};

void writer2() {
  d.store(42, std::memory_order_relaxed);
  ready2.store(true, std::memory_order_release);  // publish
}

void reader2() {
  while (!ready2.load(std::memory_order_acquire));  // spin until published
  std::cout << "Read data = " << d.load(std::memory_order_relaxed) << "\n";
}

std::atomic<int> counter2{0};
// cas: compare and swap
// compare_exchange_weak(T& expected, T desired, ...)
//
// if (x == expected) {
//   x = desired;
//   return true;
// } else {
//   expected = x;
//   return false;
// }
void increment2(int id) {
  for (int i = 0; i < 1000; ++i) {
    int expected = counter2.load(std::memory_order_relaxed);

    while (!counter2.compare_exchange_weak(expected,
                                           expected + 1,
                                           std::memory_order_release,
                                           std::memory_order_relaxed)) {
      // CAS failed
      //
      // IMPORTANT: compare_exchange_weak automatically updates the
      // |expected| to the actual current value
      std::cout << "Thread #" << id << " failed. New expected=" << expected
                << "\n";
    }
  }
}

struct Node {
  int value;
  Node* next;
};

std::atomic<Node*> head;

void push_stack(int value) {
  Node* node = new Node();
  node->value = value;
  node->next = head.load(std::memory_order_relaxed);

  while (!head.compare_exchange_weak(
      node->next, node, std::memory_order_release, std::memory_order_relaxed)) {
    // CAS failed
  }
}

void print_stack(std::atomic<Node*>& head) {
  Node* curr = head.load();

  while (curr) {
    std::cout << curr->value << "->";
    curr = curr->next;
  }

  std::cout << "\n";
}

void spawn_threads(int num_threads) {
  const int num_iters = 1 << 27;
  const int elements_per_thread = num_iters / num_threads;

  std::atomic<int> counter;

  auto work = [&]() {
    for (int i = 0; i < elements_per_thread; ++i) {
      counter.fetch_add(1, std::memory_order_relaxed);
    }
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back(work);
  }

  for (int i = 0; i < num_threads; ++i) threads[i].join();
}

void spawn_threads2() {
  const int num_iters = 1 << 27;
  const int num_threads = 4;
  const int elements_per_thread = num_iters / num_threads;

  std::array<std::atomic<int>, 4> counters{0, 0, 0, 0};
  std::atomic<int> total{0};

  auto work = [&](int idx) {
    for (int i = 0; i < elements_per_thread; ++i) {
      counters[idx].fetch_add(1, std::memory_order_relaxed);
    }
    total += counters[idx];
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back(work, i);
  }

  for (int i = 0; i < num_threads; ++i) threads[i].join();
}

struct AlignedAtomic {
  alignas(64) std::atomic<int> counter;
};

void spawn_threads3() {
  const int num_iters = 1 << 27;
  const int num_threads = 4;
  const int elements_per_thread = num_iters / num_threads;

  std::array<AlignedAtomic, 4> counters;
  std::atomic<int> total{0};

  auto work = [&](int idx) {
    for (int i = 0; i < elements_per_thread; ++i) {
      counters[idx].counter.fetch_add(1, std::memory_order_relaxed);
    }
    total += counters[idx].counter;
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back(work, i);
  }

  for (int i = 0; i < num_threads; ++i) threads[i].join();
}

void test_atomic_and_memory_order_model() {
  std::cout << "--- atomic ---\n";
  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) threads.emplace_back(increment);
  for (auto& t : threads) t.join();
  std::cout << "Final counter = " << counter << "\n";

  /*
  std::cout << "=== atomic view ===\n";
  std::atomic_ref<int> atomic_view(shared.value);
  std::atomic_view.store(42);
  std::cout << shared.value << "\n";
  */

  {
    std::cout << "--- atomic operations ---\n";

    std::atomic<int> x{10};
    int expected = 10;
    bool ok = x.compare_exchange_weak(expected, 20);
    std::cout << "Result: " << ok << ", x = " << x << "\n";

    std::thread t1(increment2, 1);
    std::thread t2(increment2, 2);
    t1.join();
    t2.join();
    std::cout << "Final = " << counter2.load() << "\n";

    std::thread t3([]() {
      for (int i = 0; i < 100; ++i) {
        push_stack(i);
      }
    });
    std::thread t4([]() {
      for (int i = 100; i < 200; ++i) {
        push_stack(i);
      }
    });
    t3.join();
    t4.join();
    print_stack(head);
  }

  {
    std::cout << "--- atomic_flag (spinlock) ---\n";
    std::thread t1(spinlock);
    std::thread t2(spinlock);
    t1.join();
    t2.join();
  }

  {
    std::cout << "--- memory order ---\n";
    std::thread t1(writer2);
    std::thread t2(reader2);
    t1.join();
    t2.join();
  }

  std::cout << "--- false sharing and cacheline alignment ---\n";
  auto start = std::chrono::high_resolution_clock::now();
  spawn_threads(1);
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "serial, num_threads=1, num_counters=1, duration="
            << std::chrono::duration<double, std::milli>(end - start).count()
            << "ms\n";

  start = std::chrono::high_resolution_clock::now();
  spawn_threads(4);
  end = std::chrono::high_resolution_clock::now();
  std::cout << "direct sharing, num_threads=4, num_counters=1, duration="
            << std::chrono::duration<double, std::milli>(end - start).count()
            << "ms\n";

  start = std::chrono::high_resolution_clock::now();
  spawn_threads2();
  end = std::chrono::high_resolution_clock::now();
  std::cout << "false sharing, num_threads=4, num_counters=4, duration="
            << std::chrono::duration<double, std::milli>(end - start).count()
            << "ms\n";

  start = std::chrono::high_resolution_clock::now();
  spawn_threads3();
  end = std::chrono::high_resolution_clock::now();
  std::cout << "no sharing, num_threads=4, num_counters=4, duration="
            << std::chrono::duration<double, std::milli>(end - start).count()
            << "ms\n";
}

// ------------
// Design Pattern: Futures & Promises
// ------------
//
// Producer ---> Promise -> Future ----> Consumer

int compute_square(int x) {
  std::cout << "Computing square of " << x << " in thread "
            << std::this_thread::get_id() << ": ";
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return x * x;
}

void test_promise_and_future() {
  std::promise<int> prom;
  std::future<int> fut = prom.get_future();
  std::thread t(
      [&prom](int x) {
        int val = compute_square(x);
        prom.set_value(val);
      },
      6);
  std::cout << "Result from promise: " << fut.get() << "\n";
  t.join();
}

void divide(std::promise<double>&& prms, double num, double denom) {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));  // simulate work
  try {
    if (denom == 0) {
      throw std::runtime_error("Exception from thread: division by zero");
    } else {
      prms.set_value(num / denom);
    }
  } catch (...) {
    prms.set_exception(std::current_exception());
  }
}

void test_promise_and_future_exception() {
  std::promise<double> prms;
  std::future<double> ftr = prms.get_future();

  double num = 42.0, denom = 0.0;
  std::thread t(divide, std::move(prms), num, denom);

  try {
    double result = ftr.get();
    std::cout << "Division result: " << result << "\n";
  } catch (std::runtime_error e) {
    std::cout << e.what() << "\n";
  }

  t.join();
}

// ------------
// Async
// ------------

void test_async() {
  std::future<int> fut1 = async(std::launch::async, compute_square, 5);
  std::cout << "Async result: " << fut1.get() << "\n";

  std::future<int> fut2 = async(std::launch::deferred, compute_square, 6);
  std::cout << "Deferred result: " << fut2.get() << "\n";
}

// ------------
// packaged_task
// ------------

void test_packaged_task() {
  std::packaged_task<int(int)> task(compute_square);
  std::future<int> fut = task.get_future();
  std::thread t2(std::move(task), 7);  // run task in separate thread
  std::cout << "Result from packaged_task: " << fut.get() << "\n";
  t2.join();
}

// ------------
// Design Pattern: Thread Pool Pattern
// ------------
class ThreadPool {
 public:
  ThreadPool(int n) {
    for (int i = 0; i < n; ++i) {
      workers.emplace_back([this]() {
        while (true) {
          std::function<void()> task;

          {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return stop || !tasks.empty(); });

            if (stop && tasks.empty()) {
              return;
            }

            task = std::move(tasks.front());
            tasks.pop();
          }

          task();
        }
      });
    }
  }

  ~ThreadPool() {
    {
      std::lock_guard<std::mutex> lock(mtx);
      stop = true;
    }
    cv.notify_all();

    for (int i = 0; i < workers.size(); ++i) {
      workers[i].join();
    }
  }

  void enqueue(std::function<void()> task) {
    {
      std::lock_guard<std::mutex> lock(mtx);
      tasks.push(task);
    }
    cv.notify_one();
  }

 private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;

  std::mutex mtx;
  std::condition_variable cv;
  bool stop = false;
};

void test_thread_pool() {
  ThreadPool pool(3);

  for (int i = 0; i < 5; ++i) {
    pool.enqueue([i]() {
      std::cout << "Task " << i << " executed by thread "
                << std::this_thread::get_id() << "\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    });
  }
}

// ------------
// Design Pattern: Monitor Object Pattern
// ------------
class BankAccount {
 private:
  int balance = 0;
  std::mutex mtx;

 public:
  void deposit(int amount) {
    std::lock_guard<std::mutex> lock(mtx);
    balance += amount;
  }

  int get_balance() {
    std::lock_guard<std::mutex> lock(mtx);
    return balance;
  }
};

void test_monitor_object_pattern() {
  BankAccount account;

  std::thread t1([&] {
    for (int i = 0; i < 10000; ++i) {
      account.deposit(1);
    }
  });

  std::thread t2([&] {
    for (int i = 0; i < 10000; ++i) {
      account.deposit(1);
    }
  });

  t1.join();
  t2.join();

  std::cout << "Account balance: " << account.get_balance() << "\n";
}

// ------------
// Design Pattern: Barrier Pattern
// ------------
void test_barrier_pattern() {
  constexpr int num_workers = 4;

  std::barrier sync(num_workers);

  std::vector<std::thread> workers;

  for (int i = 0; i < num_workers; ++i) {
    workers.emplace_back([&, i] {
      std::cout << i << " finished phase 1\n";

      sync.arrive_and_wait();

      std::cout << i << " starting phase 2\n";
    });
  }

  for (auto& t : workers) {
    t.join();
  }
}

// ------------
// Design Pattern: Read-Write Lock Pattern
// ------------

class DataCache {
 private:
  std::unordered_map<std::string, double> data;
  mutable std::shared_mutex mtx;

 public:
  void update(const std::string& symbol, double price) {
    std::unique_lock<std::shared_mutex> locker(mtx);
    data[symbol] = price;
  }

  double get(const std::string& symbol) const {
    std::shared_lock<std::shared_mutex> locker(mtx);

    auto it = data.find(symbol);

    return it == data.end() ? 0.0 : it->second;
  }
};

void test_read_write_lock_pattern() {
  DataCache cache;

  cache.update("AAPL", 200);

  std::vector<std::thread> readers;
  for (int i = 0; i < 3; ++i) {
    readers.emplace_back([&] { std::cout << cache.get("AAPL") << "\n"; });
  }

  for (auto& t : readers) {
    t.join();
  }
}

int run() {
  std::cout << "=== Thread ===\n";
  test_thread();

  std::cout << "=== Mutex & Lock guard ===\n";
  test_mutex_and_lock_guard();

  std::cout << "=== Condition Variable (Producer-Consumer Pattern) ===\n";
  test_condition_variable();

  std::cout << "=== Deadlock ===\n";
  test_deadlock();

  std::cout << "=== Atomic and Memory Order Model ===\n";
  test_atomic_and_memory_order_model();

  std::cout << "=== Promise and Future Pattern ===\n";
  test_promise_and_future();
  test_promise_and_future_exception();

  std::cout << "=== Async ===\n";
  test_async();

  std::cout << "=== packaged_task ===\n";
  test_packaged_task();

  std::cout << "=== Thread Pool Pattern ===\n";
  test_thread_pool();

  std::cout << "=== Monitor Object Pattern ===\n";
  test_monitor_object_pattern();

  std::cout << "=== Barrier Pattern ===\n";
  test_barrier_pattern();

  std::cout << "=== Read-Write Lock Pattern ===\n";
  test_read_write_lock_pattern();

  return 0;
}
}  // namespace MT
