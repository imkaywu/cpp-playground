#include "any.cpp"
#include "vector.cpp"

int main() {
#if defined(TEST_VEC)
  run_vector();
#elif defined(TEST_ANY)
  run_any();
#endif
  return 0;
}
