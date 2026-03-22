#include <iostream>
#include <stdexcept>

namespace TP {

// Function templates
template <typename T, typename U>
T clamp(T value, U low, U high) {
  using Common = decltype(value + low);

  return value < low ? low : (value > high ? high : value);
}

// Class templates
template <typename T>
class SimpleVector {
 private:
  T* data;
  size_t size;
  size_t capacity;

 public:
  SimpleVector() : data(nullptr), size(0), capacity(0) {}

  ~SimpleVector() { delete[] data; }

  void push_back(const T& value) {
    if (size == capacity) {
      capacity = capacity == 0 ? 1 : capacity * 2;
      T* new_data = new T[capacity];

      for (size_t i = 0; i < size; ++i) {
        new_data[i] = data[i];
      }

      delete[] data;
      data = new_data;
    }

    data[size++] = value;
  }

  T& operator[](size_t index) {
    if (index >= size) throw std::out_of_range("Index out of range");

    return data[index];
  }

  size_t get_size() const { return size; }
};

int run() {
  // Function templates
  clamp(1, 0.0, 2.0);

  // Class templates
  SimpleVector<int> v;
  v.push_back(10);
  v.push_back(20);
  std::cout << v[1] << "\n";

  SimpleVector<std::string> vs;
  vs.push_back("hello");
  std::cout << vs[0] << "\n";

  return 0;
}

};  // namespace TP
