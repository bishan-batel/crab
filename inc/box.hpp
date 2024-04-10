#pragma once
// #include "preamble.hpp"
#include "preamble.hpp"
#include <cassert>
#include <ostream>
#include <type_traits>
#include <utility>

namespace box {
template <typename T> struct helper {
  using ty = T *;
  using SizeType = unit;
  static constexpr SizeType DEFAULT_SIZE = unit{};
};

template <typename T> struct helper<T[]> {
  using ty = T *;
  using SizeType = usize;
  static constexpr SizeType DEFAULT_SIZE = 0;
};
}; // namespace box

/**
 * Owned Pointer (RAII) to an instance of T on the heap.
 */
template <typename T>
  requires(not std::is_const_v<T>)
class Box {
public:
private:
public:
  using Ptr = box::helper<T>::ty;
  using SizeType = box::helper<T>::SizeType;
  static constexpr auto IS_ARRAY = std::is_array_v<T>;
  static constexpr auto IS_SINGLE = not IS_ARRAY;

private:
  Ptr obj;
  SizeType size;

public:
  using Contained = std::remove_reference_t<decltype(*obj)>;

private:
  explicit Box(const Ptr from)
    requires(not IS_ARRAY)
      : obj(from), size(box::helper<T>::DEFAULT_SIZE) {}

  explicit Box(const Ptr from, SizeType length)
    requires IS_ARRAY
      : obj(from), size(length) {}

  void free() {
    if constexpr (std::is_array_v<T>) {
      delete[] obj;
    } else {
      delete obj;
    }
  }

public:
  static Box wrap_unchecked(const Ptr ref)
    requires IS_SINGLE
  {
    return Box(ref);
  };

  static Box wrap_unchecked(const Ptr ref, const SizeType length)
    requires IS_ARRAY
  {
    return Box(ref, length);
  };

  static Ptr unwrap(Box &&box)
    requires IS_SINGLE
  {
    return std::exchange(box.obj, nullptr);
  }

  static std::pair<Ptr, SizeType> unwrap(Box &&box)
    requires IS_ARRAY
  {
    return std::make_pair(
        std::exchange(box.obj, nullptr),
        std::exchange(box.size, box::helper<T>::DEFAULT_SIZE));
  }

  Box() = delete;
  Box(const Box &) = delete;

  Box(Box &&from)
      : obj(std::exchange(from.obj, nullptr)),
        size(std::exchange(from.size, box::helper<T>::DEFAULT_SIZE)) {}

  explicit Box(T val)
    requires std::is_copy_constructible_v<T> and IS_SINGLE
      : Box(new Contained(val)) {}

  explicit Box(T &&val)
    requires std::is_move_constructible_v<T> and IS_SINGLE

      : Box(new Contained(std::move(val))) {}

  ~Box() { free(); }

  void operator=(const Box &) = delete;

  void operator=(Box &&rhs)
    requires std::is_move_constructible_v<Contained> and IS_SINGLE
  {
    free();
    obj = std::exchange(rhs.obj, nullptr);
  }

  void operator=(Box &&rhs)
    requires std::is_move_constructible_v<Contained> and IS_ARRAY
  {
    free();
    obj = std::exchange(rhs.obj, nullptr);
    size = std::exchange(rhs.size, box::helper<T>::DEFAULT_SIZE);
  }

  [[nodiscard]] Contained &operator->() { return *obj; }

  [[nodiscard]] const Contained &operator->() const { return *obj; }

  [[nodiscard]] Contained &operator*() { return *obj; }
  const Contained &operator*() const { return *obj; }

  friend std::ostream &operator<<(std::ostream &os, const Box &rhs) {
    return os << *rhs;
  }

  [[nodiscard]] const Contained &operator[](const usize index) const
    requires IS_ARRAY
  {
    assert(index < size && "Index out of Bounds");
    return obj[index];
  }

  [[nodiscard]] Contained &operator[](const usize index)
    requires IS_ARRAY
  {
    assert(index < size && "Index out of Bounds");
    return obj[index];
  }

  [[nodiscard]] SizeType length() const { return size; }
};

namespace box {
/**
 * Makes a new instance of type T on the heap with given args
 */
template <typename T, typename... Args>
  requires std::is_constructible_v<T, Args...>
static Box<T> make(Args... args) {
  return Box<T>::wrap_unchecked(new T(args...));
}

/**
 * Creates a new T on the heap copied from argument
 */
template <typename T>
  requires std::is_copy_constructible_v<T>
static Box<T> create(const T &from) {
  return Box<T>::wrap_unchecked(new T(from));
}

/**
 * Moves argument into a new T on the heap
 */
template <typename T>
  requires std::is_move_constructible_v<T>
static Box<T> create(T &&from) {
  return Box<T>::wrap_unchecked(new T(std::move(from)));
}

/**
 * Creates a new array on the heap of 'count' size w/ default constructor for
 * each element
 */
template <typename T>
static Box<T[]> create_array(const usize count)
  requires std::is_default_constructible_v<T>
{
  return Box<T[]>::wrap_unchecked(new T[count](), count);
}

/**
 * Creats a new array on the heap and copies 'from' to each element
 */
template <typename T>
static Box<T[]> create_array(const usize count, const T &from)
  requires std::is_copy_constructible_v<T> and std::is_copy_assignable_v<T>
{
  auto box = Box<T[]>::wrap_unchecked(new T[count](), count);
  for (usize i = 0; i < count; i++) {
    box[i] = T(from);
  }
  return box;
}

template <typename T> static Box<T>::Ptr unwrap(Box<T> &&box) {
  return Box<T>::unwrap(std::move(box));
}

template <typename T>
static std::pair<typename Box<T>::Ptr, typename Box<T>::SizeType>
unwrap(Box<T> &&box) {
  return Box<T>::unwrap(std::move(box));
}

} // namespace box
