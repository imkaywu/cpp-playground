#include <iostream>
#include <memory>

class Any {
 private:
  struct Base {
    virtual ~Base() = default;
    virtual std::unique_ptr<Base> clone() const = 0;
    virtual const std::type_info& type() const = 0;
  };

  template <typename T>
  struct Derived : Base {
    T value;

    template <typename U>
    Derived(U&& v) : value(std::forward<U>(v)) {}

    std::unique_ptr<Base> clone() const override {
      return std::make_unique<Derived<T>>(value);
    }

    const std::type_info& type() const override { return typeid(T); }
  };

  std::unique_ptr<Base> data;

 public:
  Any() = default;

  Any(const Any& other) : data(other.data ? other.data->clone() : nullptr) {}

  Any(Any&&) noexcept = default;

  template <typename T>
  Any(T&& value) : data(std::make_unique<Derived<T>>(std::forward<T>(value))) {}

  Any& operator=(const Any& other) {
    if (this != &other) {
      data = other.data ? other.data->clone() : nullptr;
    }

    return *this;
  }

  Any& operator=(Any&& other) noexcept = default;

  bool has_value() const { return data != nullptr; }

  void reset() { data.reset(); }

  const std::type_info& type() const {
    return data ? data->type() : typeid(void);
  }

  template <typename T>
  friend T& any_cast(Any&);

  template <typename T>
  friend const T& any_cast(const Any&);
};

template <typename T>
T& any_cast(Any& a) {
  if (!a.data || a.data->type() != typeid(T)) {
    throw std::bad_cast();
  }

  return static_cast<Any::Derived<T>*>(a.data.get())->value;
}

template <typename T>
const T& any_cast(const Any& a) {
  if (!a.data || a.data->type() != typeid(T)) {
    throw std::bad_cast();
  }

  return static_cast<const Any::Derived<T>*>(a.data.get())->value;
}

int run_any() {
  Any a = 1;
  std::cout << any_cast<int>(a) << "\n";

  a = std::string("hello world");
  std::cout << any_cast<std::string>(a) << "\n";

  return 0;
}
