#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

atomic<int> counter{0};

void increment() {
  for (int i = 0; i < 1000; ++i) {
    counter.fetch_add(1, memory_order_relaxed);
  }
}

/*
struct Data { int value; };
Data shared{0};
*/

atomic<int> d{0};
atomic<bool> ready{false};

void writer() {
  d.store(42, memory_order_relaxed);
  ready.store(true, memory_order_release); // publish
}

void reader() {
  while (!ready.load(memory_order_acquire))
    ; // spin until published
  cout << "Read data = " << d.load(memory_order_relaxed) << endl;
}

struct Bad {
  atomic<int> a{0};
  atomic<int> b{0}; // adjacent - same cache line
};

Bad bad_data;

void incA() {
  for (int i = 0; i < 1'000'000; ++i)
    bad_data.a.fetch_add(1, memory_order_relaxed);
}
void incB() {
  for (int i = 0; i < 1'000'000; ++i)
    bad_data.b.fetch_add(1, memory_order_relaxed);
}

// Fallback if not available
#ifndef __cpp_lib_hardware_interference_size
#define STD_HARDWARE_DESTRUCTIVE_INTERFERENCE_SIZE 64
#else
#define STD_HARDWARE_DESTRUCTIVE_INTERFERENCE_SIZE                             \
  std::hardware_destructive_interference_size
#endif
struct alignas(STD_HARDWARE_DESTRUCTIVE_INTERFERENCE_SIZE) AlignedAtomic {
  atomic<int> value{0};
};

struct Good {
  AlignedAtomic a;
  AlignedAtomic b;
};

Good data2;

void incA2() {
  for (int i = 0; i < 1'000'000; ++i)
    data2.a.value.fetch_add(1, memory_order_relaxed);
}
void incB2() {
  for (int i = 0; i < 1'000'000; ++i)
    data2.b.value.fetch_add(1, memory_order_relaxed);
}

int main() {
  cout << "=== atomic ===\n";
  vector<thread> threads;
  for (int i = 0; i < 10; ++i)
    threads.emplace_back(increment);
  for (auto &t : threads)
    t.join();
  cout << "Final counter = " << counter << endl;

  /*
  cout << "=== atomic view ===\n";
  atomic_ref<int> atomic_view(shared.value);
  atomic_view.store(42);
  cout << shared.value << endl;
  */

  cout << "=== atomic operations ===\n";
  atomic<int> x{10};
  int expected = 10;
  bool ok = x.compare_exchange_weak(expected, 20);
  cout << "Result: " << ok << ", x = " << x << endl;

  {
    cout << "=== memory order ===\n";
    thread t1(writer);
    thread t2(reader);
    t1.join();
    t2.join();
  }

  {
    cout << "=== False sharing and cache line alignment ===\n";
    auto start = chrono::high_resolution_clock::now();
    thread t1(incA), t2(incB);
    t1.join();
    t2.join();
    auto end = chrono::high_resolution_clock::now();
    cout << "Bad took " << chrono::duration<double, milli>(end - start).count()
         << endl;
  }

  {
    auto start = chrono::high_resolution_clock::now();
    thread t1(incA2), t2(incB2);
    t1.join();
    t2.join();
    auto end = chrono::high_resolution_clock::now();
    cout << "Good took " << chrono::duration<double, milli>(end - start).count()
         << " ms\n";
  }

  return 0;
}
