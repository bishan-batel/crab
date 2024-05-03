#pragma once
#include <iostream>

#include "preamble.hpp"
#include <type_traits>
#include <utility>

#include "crab/debug.hpp"
#include "ref.hpp"

namespace crab::box {
  template<typename T>
  struct helper {
    using ty = T*;
    using const_ty = const T*;
    using SizeType = unit;
    static constexpr auto DEFAULT_SIZE = unit{};
  };

  template<typename T>
  struct helper<T[]> {
    using ty = T*;
    using const_ty = const T*;
    using SizeType = usize;
    static constexpr SizeType DEFAULT_SIZE = 0;
  };
};

/**
 * @brief Owned Pointer (RAII) to an instance of T on the heap.
 *
 * This is a replacement for std::unique_ptr, with two key differences:
 *
 * - Prevents Interior Mutability, being passed a const Box<T>& means dealing with a const T&
 * - A Box<T> has the invariant of always being non null, if it is then you messed up with move semantics.
 */
template<typename T>
  requires(not std::is_const_v<T>)
class Box {
public:
  using MutPtr = typename crab::box::helper<T>::ty;
  using ConstPtr = typename crab::box::helper<T>::const_ty;
  using SizeType = typename crab::box::helper<T>::SizeType;
  static constexpr auto IS_ARRAY = std::is_array_v<T>;
  static constexpr auto IS_SINGLE = not IS_ARRAY;

private:
  MutPtr obj;
  SizeType size;

public:
  using Contained = std::remove_reference_t<decltype(*obj)>;

private:
  explicit Box(const MutPtr from)
    requires(not IS_ARRAY)
    : obj(from), size(crab::box::helper<T>::DEFAULT_SIZE) {}

  explicit Box(const MutPtr from, SizeType length)
    requires IS_ARRAY
    : obj(from), size(length) {}

  // ReSharper disable once CppMemberFunctionMayBeConst
  void drop() {
    if constexpr (std::is_array_v<T>) {
      delete[] obj;
    } else {
      delete obj;
    }
  }

public:
  /**
   * @brief Wraps pointer with RAII
   *
   * This function has no way of knowing if the pointer passed is actually on
   * the heap or if something else has ownership, therefor 'unchecked' and it is
   * the responsibility of the caller to make sure.
   */
  static Box wrap_unchecked(const MutPtr ref)
    requires IS_SINGLE {
    return Box(ref);
  };

  /**
   * @brief Wraps pointer to an array with RAII
   *
   * This function has no way of knowing if the pointer passed is actually on
   * the heap or if something else has ownership, therefor 'unchecked' and it is
   * the responsibility of the caller to make sure.
   */
  static Box wrap_unchecked(const MutPtr ref, const SizeType length)
    requires IS_ARRAY {
    return Box(ref, length);
  };

  /**
   * @brief Reliquenshes ownership & opts out of RAII, giving you the raw
   * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
   */
  static MutPtr unwrap(Box box) requires IS_SINGLE {
    const auto ptr = box.raw_ptr();
    box.obj = nullptr;
    return ptr;
  }

  /**
   * @brief Reliquenshes ownership & opts out of RAII, giving you the raw
   * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
   */
  static std::pair<MutPtr, SizeType> unwrap(Box box) requires IS_ARRAY {
    return std::make_pair(
      std::exchange(box.obj, nullptr),
      std::exchange(box.size, crab::box::helper<T>::DEFAULT_SIZE)
    );
  }

  Box() = delete;

  Box(const Box &) = delete;

  Box(Box &&from) noexcept
    : obj(std::exchange(from.obj, nullptr)),
      size(std::exchange(from.size, crab::box::helper<T>::DEFAULT_SIZE)) {}

  // ReSharper disable once CppNonExplicitConvertingConstructor
  template<typename Derived> requires std::is_base_of_v<T, Derived> and IS_SINGLE
  Box(Box<Derived> &&from)
    : Box(from.__release_for_derived()) {
    debug_assert(obj != nullptr, "Invalid Box, moved from invalid box.");
  }

  explicit Box(T val)
    requires std::is_copy_constructible_v<T> and IS_SINGLE
    : Box(new Contained(val)) {}

  explicit Box(T &&val)
    requires std::is_move_constructible_v<T> and IS_SINGLE

    : Box(new Contained(std::move(val))) {}

  ~Box() { drop(); }

  // ReSharper disable once CppNonExplicitConversionOperator
  __always_inline operator Contained&() { return *raw_ptr(); } // NOLINT(*-explicit-constructor)

  // ReSharper disable once CppNonExplicitConversionOperator
  __always_inline operator const Contained&() const { return *raw_ptr(); } // NOLINT(*-explicit-constructor)

  // ReSharper disable once CppNonExplicitConversionOperator
  __always_inline operator Ref<Contained>() const {
    return crab::ref::from_ptr_unchecked(raw_ptr());
  }

  __always_inline operator RefMut<Contained>() {
    return crab::ref::from_ptr_unchecked(raw_ptr());
  }

  // NOLINT(*-explicit-constructor)
  void operator=(const Box &) = delete;

  void operator=(Box &&rhs) noexcept
    requires std::is_move_constructible_v<Contained> and IS_SINGLE {
    drop();
    obj = std::exchange(rhs.obj, nullptr);
  }

  void operator=(Box &&rhs) noexcept
    requires std::is_move_constructible_v<Contained> and IS_ARRAY {
    drop();
    obj = std::exchange(rhs.obj, nullptr);
    size = std::exchange(rhs.size, crab::box::helper<T>::DEFAULT_SIZE);
  }

  [[nodiscard]] __always_inline MutPtr operator->() { return as_ptr(); }

  [[nodiscard]] __always_inline ConstPtr operator->() const { return as_ptr(); }

  [[nodiscard]] __always_inline Contained& operator*() { return *as_ptr(); }

  __always_inline const Contained& operator*() const { return *as_ptr(); }

  friend std::ostream& operator<<(std::ostream &os, const Box &rhs) {
    return os << *rhs;
  }

  [[nodiscard]] const Contained& operator[](const usize index) const
    requires IS_ARRAY {
    debug_assert(index < size, "Index out of Bounds");
    return as_ptr()[index];
  }

  [[nodiscard]] Contained& operator[](const usize index)
    requires IS_ARRAY {
    debug_assert(index < size, "Index out of Bounds");
    return as_ptr()[index];
  }

  [[nodiscard]] SizeType length() const { return size; }

  [[nodiscard]] MutPtr as_ptr() { return raw_ptr(); }

  [[nodiscard]] ConstPtr as_ptr() const { return raw_ptr(); }

  [[nodiscard]] MutPtr __release_for_derived() { return std::exchange(obj, nullptr); }

private:
  MutPtr raw_ptr() {
    debug_assert(obj != nullptr, "Invalid Use of Moved Box<T>.");
    return obj;
  }

  ConstPtr raw_ptr() const {
    debug_assert(obj != nullptr, "Invalid Use of Moved Box<T>.");
    return obj;
  }
};

namespace crab {
  /**
   * @brief Makes a new instance of type T on the heap with given args
   */
  template<typename T, typename... Args>
    requires std::is_constructible_v<T, Args...>
  static Box<T> make_box(Args &&... args) {
    return Box<T>::wrap_unchecked(new T{std::forward<Args>(args)...});
  }

  /**
   * @brief Makes a new instance of type T on the heap with given args
   */
  template<typename T, typename V>
    requires std::is_convertible_v<T, V>
  static Box<T> make_box(V &&from) {
    return Box<T>::wrap_unchecked(new T{static_cast<T>(from)});
  }

  /**
   * @brief Creates a new array on the heap of 'count' size w/ default constructor for
   * each element
   */
  template<typename T>
  static Box<T[]> make_boxxed_array(const usize count)
    requires std::is_default_constructible_v<T> {
    return Box<T[]>::wrap_unchecked(new T[count](), count);
  }

  /**
   * @brief Creates a new array on the heap and copies 'from' to each element
   */
  template<typename T>
  static Box<T[]> make_boxxed_array(const usize count, const T &from)
    requires std::is_copy_constructible_v<T> and std::is_copy_assignable_v<T> {
    auto box = Box<T[]>::wrap_unchecked(new T[count](), count);
    for (usize i = 0; i < count; i++) {
      box[i] = T(from);
    }
    return box;
  }

  /**
   * @brief Reliquenshes ownership & opts out of RAII, giving you the raw
   * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
   */
  template<typename T> requires Box<T>::IS_SINGLE
  static typename Box<T>::MutPtr release(Box<T> box) {
    return Box<T>::unwrap(std::move(box));
  }

  /**
   * @brief Reliquenshes ownership & opts out of RAII, giving you the raw
   * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
   */
  template<typename T> requires Box<T>::IS_ARRAY
  static std::pair<typename Box<T>::MutPtr, typename Box<T>::SizeType>
  release(Box<T> box) {
    return Box<T>::unwrap(std::move(box));
  }
}
