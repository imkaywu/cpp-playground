#include <iostream>
#include <string_view>

// 1. constructor is private
// 2. singleton access function is static
// 3. copy and move operations are private
class Logger {
 public:
  Logger(const Logger&) = delete;
  Logger(Logger&&) = delete;

  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = delete;

  static Logger& Instance() {
    static Logger logger{};
    return logger;
  }

  void Info(std::string_view msg) { std::cout << "Info: " << msg << std::endl; }

  void Error(std::string_view msg) {
    std::cout << "Error: " << msg << std::endl;
  }

 private:
  Logger() = default;
};
