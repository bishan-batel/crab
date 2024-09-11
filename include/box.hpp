#pragma once
#include <iostream>

#include <type_traits>
#include <utility>
#include "crab/type_traits.hpp"
#include "preamble.hpp"

#include "crab/debug.hpp"
#include "ref.hpp"

namespace crab::box {
  template<typename T>
  struct helper {
    using ty = T *;
    using const_ty = const T *;
    using SizeType = unit;
    static constexpr unit DEFAULT_SIZE{};
  };

  template<typename T>
  struct helper<T[]> {
    using ty = T *;
    using const_ty = const T *;
    using SizeType = usize;
    static constexpr SizeType DEFAULT_SIZE = 0;
  };
}; // namespace crab::box

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
  // using SizeType = typename crab::box::helper<T>::SizeType;
  // static constexpr auto IS_ARRAY = crab::is_complete_type<T> and std::is_array_v<T>;
  // static constexpr auto IS_SINGLE = not IS_ARRAY;

private:
  MutPtr obj;
  // SizeType size;

public:
  using Contained = std::remove_reference_t<decltype(*obj)>;

private:
  explicit Box(const MutPtr from)
      // requires(not IS_ARRAY)
      :
      obj(from)
  // , size(crab::box::helper<T>::DEFAULT_SIZE)
  {}

  // explicit Box(const MutPtr from, SizeType length)
  //  requires IS_ARRAY
  //    : obj(from), size(length) {}

  // ReSharper disable once CppMemberFunctionMayBeConst
  auto drop() -> void {
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
  static auto wrap_unchecked(const MutPtr ref) -> Box
    // requires IS_SINGLE
  {
    return Box(ref);
  };

  /**
   * @brief Wraps pointer to an array with RAII
   *
   * This function has no way of knowing if the pointer passed is actually on
   * the heap or if something else has ownership, therefor 'unchecked' and it is
   * the responsibility of the caller to make sure.
   */
  //  static auto wrap_unchecked(const MutPtr ref, const SizeType length) -> Box
  //   requires IS_ARRAY
  // {
  //   return Box(ref, length);
  // };

  /**
   * @brief Reliquenshes ownership & opts out of RAII, giving you the raw
   * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
   */
  static auto unwrap(Box box) -> MutPtr
    // requires IS_SINGLE
  {
    const auto ptr = box.raw_ptr();
    box.obj = nullptr;
    return ptr;
  }

  /**
   * @brief Reliquenshes ownership & opts out of RAII, giving you the raw
   * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
   */
  //  static auto unwrap(Box box) -> std::pair<MutPtr, SizeType>
  //   requires IS_ARRAY
  // {
  //   return std::make_pair(std::exchange(box.obj, nullptr), std::exchange(box.size,
  //   crab::box::helper<T>::DEFAULT_SIZE));
  // }
  //
  Box() = delete;

  Box(const Box &) = delete;

  Box(Box &&from) noexcept :
      obj(std::exchange(from.obj, nullptr))
  // , size(std::exchange(from.size, crab::box::helper<T>::DEFAULT_SIZE))
  {}

  // ReSharper disable once CppNonExplicitConvertingConstructor
  template<typename Derived>
    requires std::is_base_of_v<T, Derived> 
    // and IS_SINGLE
  Box(Box<Derived> &&from) // NOLINT
      : Box{Box<Derived>::unwrap(std::forward<Box<Derived>>(from))} {
    debug_assert(obj != nullptr, "Invalid Box, moved from invalid box.");
  }

  explicit Box(T val)
    requires crab::is_complete_type<T> and std::is_copy_constructible_v<T> 
      : Box(new Contained(val)) {}

  explicit Box(T &&val)
    requires crab::is_complete_type<T> and std::is_move_constructible_v<T> 
      : Box(new Contained(std::move(val))) {}

  ~Box() { drop(); }

  operator Contained &() { return *raw_ptr(); } // NOLINT(*-explicit-constructor)

  operator const Contained &() const { return *raw_ptr(); } // NOLINT(*-explicit-constructor)

  operator Ref<Contained>() const { // NOLINT(*-explicit-constructor)
    return crab::ref::from_ptr_unchecked(raw_ptr());
  }

  operator RefMut<Contained>() { // NOLINT(*-explicit-constructor)
    return crab::ref::from_ptr_unchecked(raw_ptr());
  }

  auto operator=(const Box &) -> void = delete;

  auto operator=(Box &&rhs) noexcept -> Box &
    // requires IS_SINGLE
  {
    if (&rhs == this) return *this;

    drop();
    obj = std::exchange(rhs.obj, nullptr);

    return *this;
  }

  template<typename Derived>
    requires std::is_base_of_v<T, Derived> and (not std::is_same_v<T, Derived>)
             auto operator=(Box<Derived> &&rhs) noexcept -> Box &
               // requires IS_SINGLE
  {
    if (&rhs == this) return *this;

    drop();
    obj = static_cast<T *>(Box<Derived>::unwrap(std::forward<Box<Derived>>(rhs)));

    return *this;
  }

  // auto operator=(Box rhs) noexcept -> Box &
  //   requires IS_ARRAY
  // {
  //   if (&rhs == this) return *this;
  //   drop();
  //   obj = std::exchange(rhs.obj, nullptr);
  //   size = std::exchange(rhs.size, crab::box::helper<T>::DEFAULT_SIZE);
  //
  //   return *this;
  // }

  [[nodiscard]] auto operator->() -> MutPtr { return as_ptr(); }

  [[nodiscard]] auto operator->() const -> ConstPtr { return as_ptr(); }

  [[nodiscard]] auto operator*() -> Contained & { return *as_ptr(); }

  auto operator*() const -> const Contained & { return *as_ptr(); }

  friend auto operator<<(std::ostream &os, const Box &rhs) -> std::ostream & { return os << *rhs; }

  // [[nodiscard]] auto operator[](const usize index) const -> const Contained &
  //   requires IS_ARRAY
  // {
  //   debug_assert(index < size, "Index out of Bounds");
  //   return as_ptr()[index];
  // }
  //
  // [[nodiscard]] auto operator[](const usize index) -> Contained &
  //   requires IS_ARRAY
  // {
  //   debug_assert(index < size, "Index out of Bounds");
  //   return as_ptr()[index];
  // }

  // [[nodiscard]] auto length() const -> SizeType { return size; }

  [[nodiscard]] auto as_ptr() -> MutPtr { return raw_ptr(); }

  [[nodiscard]] auto as_ptr() const -> ConstPtr { return raw_ptr(); }

private:
  auto raw_ptr() -> MutPtr {
#if DEBUG
    debug_assert(obj != nullptr, "Invalid Use of Moved Box<T>.");
#endif
    return obj;
  }

  auto raw_ptr() const -> ConstPtr {
#if DEBUG
    debug_assert(obj != nullptr, "Invalid Use of Moved Box<T>.");
#endif
    return obj;
  }
};

namespace crab {
  /**
   * @brief Makes a new instance of type T on the heap with given args
   */
  template<typename T, typename... Args>
    requires std::is_constructible_v<T, Args...>
  static auto make_box(Args &&...args) -> Box<T> {
    return Box<T>::wrap_unchecked(new T{std::forward<Args>(args)...});
  }

  /**
   * @brief Makes a new instance of type T on the heap with given args
   */
  template<typename T, typename V>
    requires std::is_convertible_v<T, V> and (std::is_integral_v<T> or std::is_floating_point_v<T>)
  static auto make_box(V &&from) -> Box<T> {
    return Box<T>::wrap_unchecked(new T{static_cast<T>(from)});
  }

  // /**
  //  * @brief Creates a new array on the heap of 'count' size w/ default constructor for
  //  * each element
  //  */
  // template<typename T>
  // static auto make_boxxed_array(const usize count) -> Box<T[]>
  //   requires std::is_default_constructible_v<T>
  // {
  //   return Box<T[]>::wrap_unchecked(new T[count](), count);
  // }
  //
  // /**
  //  * @brief Creates a new array on the heap and copies 'from' to each element
  //  */
  // template<typename T>
  // static auto make_boxxed_array(const usize count, const T &from) -> Box<T[]>
  //   requires std::is_copy_constructible_v<T> and std::is_copy_assignable_v<T>
  // {
  //   auto box = Box<T[]>::wrap_unchecked(new T[count](), count);
  //   for (usize i = 0; i < count; i++) {
  //     box[i] = T(from);
  //   }
  //   return box;
  // }
  //
  /**
   * @brief Reliquenshes ownership & opts out of RAII, giving you the raw
   * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
   */
  template<typename T>
    // requires Box<T>::IS_SINGLE
  static auto release(Box<T> box) -> typename Box<T>::MutPtr {
    return Box<T>::unwrap(std::move(box));
  }

  // /**
  //  * @brief Reliquenshes ownership & opts out of RAII, giving you the raw
  //  * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
  //  */
  // template<typename T>
  //   requires Box<T>::IS_ARRAY
  // static auto release(Box<T> box) -> std::pair<typename Box<T>::MutPtr, typename Box<T>::SizeType> {
  //   return Box<T>::unwrap(std::move(box));
  // }
} // namespace crab
