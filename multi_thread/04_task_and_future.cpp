#include <chrono>
#include <future>
#include <iostream>
#include <thread>

using namespace std;

int compute_square(int x) {
  cout << "Computing square of " << x << " in thread " << this_thread::get_id()
       << ": ";
  this_thread::sleep_for(chrono::seconds(1));
  return x * x;
}

int main() {
  cout << "Main thread id: " << this_thread::get_id() << endl;

  // 1. async + future
  future<int> fut1 = async(std::launch::async, compute_square, 5);
  cout << "Result from async: " << fut1.get() << endl; // block until ready

  // 2. promise + future
  promise<int> prom;
  future<int> fut2 = prom.get_future();
  thread t(
      [&prom](int x) {
        int val = compute_square(x);
        prom.set_value(val);
      },
      6);
  cout << "Result from promise: " << fut2.get() << endl;
  t.join();

  // 3. packaged_task
  packaged_task<int(int)> task(compute_square);
  future<int> fut3 = task.get_future();
  thread t2(std::move(task), 7); // run task in separate thread
  cout << "Result from packaged_task: " << fut3.get() << endl;
  t2.join();

  // 4. async lazy evaluation
  future<int> fut4 = async(std::launch::deferred, compute_square, 8);
  cout << "Deferred async has not run yet\n";
  cout << "Result from deferred async: " << fut4.get() << endl; // runs now

  return 0;
}
