#include <chrono>
#include <iostream>
#include <thread>

using namespace std;

// -------------------------------
// Thread-local variable
// Each thread gets its own copy
// -------------------------------
thread_local int thread_counter = 0;

void worker(int id, const string &name) {
  ++thread_counter;

  cout << "Thread " << id << " (" << name << ") running on ID "
       << this_thread::get_id() << ", thread_local counter = " << thread_counter
       << endl;

  this_thread::sleep_for(chrono::milliseconds(500));
  cout << "Thread " << id << " done\n";
}

void increment(int &x) {
  for (int i = 0; i < 5; ++i) {
    ++x;
    this_thread::sleep_for(chrono::milliseconds(100));
  }
}

int main() {
  cout << "Main thread ID: " << this_thread::get_id() << "\n\n";

  thread t1(worker, 1, "Alpha");
  thread t2(worker, 2, "Beta");

  int shared_value = 0;
  thread t3(increment, ref(shared_value));

  cout << "\nBefore joining, joinable states:\n";
  cout << "t1.joinable(): " << t1.joinable() << endl;
  cout << "t2.joinable(): " << t2.joinable() << endl;
  cout << "t3.joinable(): " << t3.joinable() << endl;

  t1.join();
  t2.join();
  t3.detach();

  cout << "\nAfter joining/detaching:\n";
  cout << "t1.joinable(): " << t1.joinable() << endl;
  cout << "t2.joinable(): " << t2.joinable() << endl;
  cout << "t3.joinable(): " << t3.joinable() << endl;

  this_thread::sleep_for(chrono::seconds(1));

  cout << "\nShared value after detached thread increment: " << shared_value
       << endl;

  cout << "\nMain thread done.\n";

  return 0;
}
