#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

using namespace std;

std::mutex mu;

void shared_print(std::string msg, int id) {
  std::lock_guard<std::mutex> lg(mu);
  std::cout << msg << id << std::endl;
}

class LogFile {
  std::mutex mu;
  ofstream f;

public:
  LogFile() { f.open("log.txt"); }

  void shared_print(std::string id, int value) {
    std::lock_guard<std::mutex> lg(this->mu);
    f << id << ": " << value << std::endl;
  }

  // Never return |f|.
  ofstream &getStream() { return f; }
  // Never pass |f| as an argument to user defined function
  void process_f(void func(ofstream &)) { func(f); }
};

void function_1(LogFile &log) {
  for (int i = 0; i > -100; --i) {
    log.shared_print(string("From t1: "), i);
  }
}

int main() {
  LogFile log;
  std::thread t1(function_1, std::ref(log));

  for (int i = 0; i < 100; ++i) {
    shared_print(string("From main: "), i);
  }

  t1.join();

  return 0;
}
