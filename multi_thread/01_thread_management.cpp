#include <iostream>
#include <thread>

using namespace std;

void function_1() { std::cout << "Beauty is only skin-deep" << std::endl; }

class Functor {
public:
  Functor() = default;
  Functor(const Functor &) { std::cout << "object copied" << std::endl; }
  Functor(Functor &&) { std::cout << "object moved" << std::endl; }

  void operator()(std::string &msg) {
    std::cout << "t1 says: " << msg << std::endl;
    msg = "here is another message";
  }
};

int main() {
  std::string msg = "here is some message";
  Functor functor;
  std::thread t1(std::move(functor), std::ref(msg)); // t1 starts running

  // std::thread t2 = t1; // thread not copyable
  std::thread t2 = std::move(t1); //

  try {
    for (int i = 0; i < 10; ++i) {
      std::cout << "from main: " << i << std::endl;
    }
  } catch (...) {
    t2.join();
    throw;
  }

  t2.join();

  std::cout << "main says: " << msg << std::endl;
  return 0;
}
