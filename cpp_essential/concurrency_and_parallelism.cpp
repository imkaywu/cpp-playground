#include <chrono>
#include <condition_variable>
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
  Functor(const Functor &) { std::cout << "functor copied\n"; }
  Functor(Functor &&) { std::cout << "functor moved\n"; }

  void operator()(std::string &msg) {
    std::cout << "t1 says: " << msg << "\n";
    msg = "here is another message";
  }
};

// Thread-local variable: each thread gets its own copy
thread_local int thread_counter = 0;

void worker(int id, const std::string &name) {
  ++thread_counter;

  std::cout << "Thread " << id << " (" << name << ") running on ID "
            << std::this_thread::get_id()
            << ", thread_local counter = " << thread_counter << "\n";

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  std::cout << "Thread " << id << " done\n";
}

void increment(int &x) {
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
    for (int i = 0; i < 10; ++i) {
      std::cout << "from main: " << i << "\n";
    }
  } catch (...) {
    t2.join();
    throw;
  }

  t2.join();

  std::cout << "main says: " << msg << "\n";

  std::cout << "\n--- Variable in Thread ---\n";
  std::cout << "Main thread ID: " << std::this_thread::get_id() << "\n\n";

  std::thread t3(worker, 1, "Alpha");
  std::thread t4(worker, 2, "Beta");

  int shared_value = 0;
  std::thread t5(increment, std::ref(shared_value));

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

  std::this_thread::sleep_for(std::chrono::seconds(1));

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

int test_mutex_and_lock_guard() {
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
  for (auto &t : readers) t.join();
  for (auto &t : writers) t.join();

  std::cout << "--- 4. unique_lock ---\n";
  unique_lock_example();
}

// ------------
// Condition variable
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

void worker_wait() {
  std::unique_lock<std::mutex> lock(mtx);
  std::cout << "Worker waiting...\n";
  // cv.wait(lock, predicate): put thread to sleep until predicate is true
  cv.wait(lock, [] { return ready; });  // waits until ready==true
  std::cout << "Workder proceeds\n";
}

std::deque<int> q;
std::condition_variable cond;

void produce() {
  int count = 10;
  while (count > 0) {
    std::unique_lock<std::mutex> locker(mtx);
    q.push_front(count);
    locker.unlock();
    cond.notify_one();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    count--;
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

int run() {
  std::cout << "=== Thread ===\n";
  test_thread();

  std::cout << "=== Mutex & Lock guard ===\n";
  test_mutex_and_lock_guard();

  std::cout << "=== Condition variable ===\n";
  std::thread waiter(worker_wait);
  std::thread signaler(notifier);
  waiter.join();
  signaler.join();

  std::thread producer(produce);
  std::thread consumer(consume);
  producer.join();
  consumer.join();

  return 0;
}
}  // namespace MT
