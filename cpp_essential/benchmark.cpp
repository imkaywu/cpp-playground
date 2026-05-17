#include <chrono>
#include <iostream>
#include <string>
#include <vector>

template <typename Alloc>
void benchmark_vector_push(const std::string& name,
                           Alloc alloc,
                           std::size_t count) {
  using Vector = std::vector<int, Alloc>;

  auto start = std::chrono::high_resolution_clock::now();

  {
    Vector vec(alloc);

    // calls `alloc.allocate(count)`
    vec.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
      vec.push_back(static_cast<int>(i));
    }

    volatile int sink = vec.back();
    (void)sink;
  }

  auto end = std::chrono::high_resolution_clock::now();

  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  std::cout << name << ": " << duration.count() << " us\n";
}
