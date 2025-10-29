#include <chrono>
#include <deque>
#include <iostream>
#include <mutex>
#include <thread>

std::deque<int> q;
std::mutex mu;
std::condition_variable cond;

void function_1() {
  int count = 10;
  while (count > 0) {
    std::unique_lock<std::mutex> locker(mu);
    q.push_front(count);
    locker.unlock();
    cond.notify_one(); // Notify one thread waiting on this thread.
    std::this_thread::sleep_for(std::chrono::seconds(1));
    count--;
  }
}

void function_2() {
  int data = 0;
  while (data != 1) {
    std::unique_lock<std::mutex> locker(mu);
    cond.wait(locker, []() { return !q.empty(); });
    data = q.back();
    q.pop_back();
    locker.unlock();
    std::cout << "t2 got a value from t1: " << data << std::endl;
  }
}

int main() {
  return 0;
}
