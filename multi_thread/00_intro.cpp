#include <iostream>
#include <thread>

using namespace std;

void function_1() { std::cout << "Beauty is only skin-deep" << std::endl; }

int main() {
  std::thread t1(function_1); // t1 starts running
  // t1.join(); // main thread waits for t1 to finish
  t1.detach(); // t1 will be on its own - daemon process

  // Checks if the thread object owns a running (joinable) thread
  if (t1.joinable()) {
    t1.join(); // crash
  } else {
    std::cout << "t1 not joinable" << std::endl;
  }

  return 0;
}
