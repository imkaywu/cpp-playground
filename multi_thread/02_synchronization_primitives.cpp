#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

using namespace std;

// Global shared state
int shared_data = 0;

mutex mtx;
// same thread can lock it multiple times (used in recursion)
recursive_mutex re_mtx;
// allows multiple readers, but a single writer to hold it
shared_mutex rw_mtx;

condition_variable cv;
bool ready = false;

atomic<int> atomic_counter(0);
atomic_flag atomic_flag_lock = ATOMIC_FLAG_INIT;

// 1. mutex + lock_guard
// lock_guard: RAII wrapper for mutex
void increment_with_mutex() {
  lock_guard<mutex> lock(mtx); // auto unlock at scope exit
  shared_data++;
  cout << "increment_with_mutex: " << shared_data << endl;
}

// 2. recursive_mutex
void recursive_func(int n) {
  if (n <= 0)
    return;
  re_mtx.lock();
  cout << "re_mtx depth: " << n << endl;
  recursive_func(n - 1);
  re_mtx.unlock();
}

// 3. shared_mutex
void reader(int id) {
  shared_lock<shared_mutex> lock(rw_mtx);
  cout << "Reader " << id << " reads shared_data = " << shared_data << endl;
}

void writer(int id) {
  unique_lock<shared_mutex> lock(rw_mtx);
  shared_data++;
  cout << "Writer " << id << " updates shared_data = " << shared_data << endl;
}

// 4. unique_lock + defer/adopt/try_lock
// unique_lock: more flexible lock
void unique_lock_example() {
  unique_lock<mutex> lock(mtx, defer_lock); // not locked yet
  // try_lock: non-blocking
  if (lock.try_lock()) {
    cout << "unique_lock acquired using try_lock()\n";
  }
  lock.unlock();

  mtx.lock();
  unique_lock<mutex> adopted_lock(mtx, adopt_lock);
  cout << "unique_lock adopted existing lock\n";
  // lock auto-unlocked when goes out of scope
}

// 5. condition_variable (wait/notify)
void worker_wait() {
  unique_lock<mutex> lock(mtx);
  cout << "Worker waiting...\n";
  // cv.wait(lock, predicate): put thread to sleep until predicate is true
  cv.wait(lock, [] { return ready; }); // waits until ready==true
  cout << "Workder proceeds\n";
}

void notifier() {
  this_thread::sleep_for(chrono::milliseconds(500));
  {
    lock_guard<mutex> lock(mtx);
    ready = true;
    cout << "Notifier: setting ready = true\n";
  }
  cv.notify_one(); // wake up one waiting thread
}

// 6. atomic and atomic_flag
void atomic_example() {
  for (int i = 0; i < 1000; ++i) {
    atomic_counter.fetch_add(1, memory_order_relaxed);
  }
}

void spinlock_example() {
  // Sets the flag to true and return previous value:
  // - If the lock is acquired: by another thread, keeps spinning
  // - If the lock is released: acquire the lock, set to true, return false
  while (atomic_flag_lock.test_and_set(memory_order_acquire)) {
    // busy-wait (spin)
  }
  cout << "Atomic flag lock acquired by thread " << this_thread::get_id()
       << endl;
  this_thread::sleep_for(chrono::milliseconds(100));
  atomic_flag_lock.clear(memory_order_release);
}

int main() {
  cout << "=== 1. Basic mutex and lock_guard ===\n";
  thread t1(increment_with_mutex);
  thread t2(increment_with_mutex);
  t1.join();
  t2.join();

  cout << "=== 2. recursive_mutex ===\n";
  recursive_func(3);

  cout << "=== 3. shared_mutex (readers/writers) ===\n";
  vector<thread> readers, writers;
  for (int i = 0; i < 3; ++i)
    readers.emplace_back(reader, i);
  for (int i = 0; i < 2; ++i)
    writers.emplace_back(writer, i);
  for (auto &t : readers)
    t.join();
  for (auto &t : writers)
    t.join();

  cout << "=== 4. unique_lock ===\n";
  unique_lock_example();

  cout << "=== 5. condition_variable ===\n";
  thread waiter(worker_wait);
  thread signaler(notifier);
  waiter.join();
  signaler.join();

  cout << "=== 6. atomic operations ===\n";
  vector<thread> atom_threads;
  for (int i = 0; i < 4; ++i)
    atom_threads.emplace_back(atomic_example);
  for (auto &t : atom_threads)
    t.join();
  cout << "Atomic counter = " << atomic_counter.load() << endl;

  cout << "=== 7. atomic_flag (spinlock) ===\n";
  thread s1(spinlock_example);
  thread s2(spinlock_example);
  s1.join();
  s2.join();

  return 0;
}
