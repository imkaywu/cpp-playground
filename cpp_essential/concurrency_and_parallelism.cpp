#include <chrono>
#include <iostream>
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

// ------------
// Lock guards
// ------------

// ------------
// Condition variable
// ------------

int run() {
  std::cout << "=== Thread ===\n";
  test_thread();

  return 0;
}
}  // namespace MT
