#include <iostream>
#include <string>

#include "vector.cpp"

template <typename T>
void PrintVector(const Vector<T>& vector) {
  for (size_t i = 0; i < vector.Size(); ++i) {
    std::cout << vector[i].x << " " << vector[i].y << " " << vector[i].z
              << std::endl;
  }
  std::cout << "--------------------\n";
}

struct Vector3 {
  float x = 0.0;
  float y = 0.0;
  float z = 0.0;

  Vector3() { std::cout << "Constructor\n"; }
  // NOTE: only constructors take base initializers
  Vector3(float scalar) : x(scalar), y(scalar), z(scalar) {
    std::cout << "Constructor scalar\n";
  }
  Vector3(float x, float y, float z) : x(x), y(y), z(z) {
    std::cout << "Constructor xyz\n";
  }

  Vector3(const Vector3& other) : x(other.x), y(other.y), z(other.z) {
    std::cout << "Copy constructor\n";
  }

  Vector3(Vector3&& other) : x(other.x), y(other.y), z(other.z) {
    std::cout << "Move constructor\n";
  }

  Vector3& operator=(const Vector3& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    std::cout << "Copy assignment\n";

    // NOTE: remember to return the instance itself.
    return *this;
  }

  Vector3& operator=(Vector3&& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    std::cout << "Move assignment\n";

    // NOTE: remember to return the instance itself.
    return *this;
  }

  ~Vector3() { std::cout << "Destructor\n"; }
};

int main() {
  {
    Vector<Vector3> vector;
    vector.PushBack(Vector3());
    vector.PushBack(Vector3(1.0));
    vector.PushBack(Vector3(1.0, 2.0, 3.0));

    PrintVector(vector);
  }

  {
    Vector<Vector3> vector;
    vector.EmplaceBack();
    vector.EmplaceBack(1.0);
    vector.EmplaceBack(1.0, 2.0, 3.0);

    PrintVector(vector);
  }

  return 0;
}
