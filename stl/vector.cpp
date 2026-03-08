#include <cstddef>
#include <utility>

// TODO:
//  - add destructor, PopBack, Clear
//  - be mindful of a potential bug when deleting |data|

template <typename T>
class Vector {
 public:
  Vector() { Realloc(2); }

  void PushBack(const T& value) {
    if (size >= capacity) {
      Realloc(2 * capacity);
    }

    // copy assignment
    data[size++] = value;
  }

  void PushBack(T&& value) {
    if (size >= capacity) {
      Realloc(2 * capacity);
    }

    // move assignment
    data[size++] = std::move(value);
  }

  template <typename... Args>
  T& EmplaceBack(Args&&... args) {
    if (size >= capacity) {
      Realloc(2 * capacity);
    }

    // NOTE: this line does 1) create a temp instance on stack and 2) move to
    // specified position, which is not construct in place.
    // data[size] = T(std::forward<Args>(args)...);

    // NOTE: use placement new operator
    new (&data[size]) T(std::forward<Args>(args)...);

    return data[size++];
  }

  void Realloc(size_t new_capacity) {
    T* new_data = new T[new_capacity];

    // NOTE: in case we're downsizing the vector.
    if (new_capacity < size) {
      size = new_capacity;
    }

    // NOTE: why not using memcpy?
    for (size_t i = 0; i < size; ++i) {
      // new_data[i] = data[i];  // NOTE: use copy assignment

      new_data[i] = std::move(data[i]);  // NOTE: use move assignment
    }

    // NOTE: delete[] nullptr is no-op
    delete[] data;
    data = new_data;
    capacity = new_capacity;
  }

  const T& operator[](size_t idx) const {
    if (idx >= size) {
      // assert
    }
    return data[idx];
  }

  T& operator[](size_t idx) {
    if (idx >= size) {
      // assert
    }
    return data[idx];
  }

  size_t Size() const { return size; }

 private:
  // NOTE: must initialize members, otherwise garbage pointer/value.
  T* data = nullptr;
  size_t size = 0;
  size_t capacity = 0;
};
