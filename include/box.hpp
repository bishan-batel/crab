#pragma once
#include <concepts>
#include <iostream>

#include <type_traits>
#include <utility>
#include "crab/type_traits.hpp"
#include "preamble.hpp"

#include "crab/debug.hpp"
#include "ref.hpp"

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
  using MutPtr = T *;
  using ConstPtr = const T *;

private:
  MutPtr obj;
  // SizeType size;

public:
  using Contained = std::remove_reference_t<decltype(*obj)>;

private:
  explicit Box(const MutPtr from) : obj(from) {} // NOLINT

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
  static auto wrap_unchecked(const MutPtr ref) -> Box { return Box(ref); }; // NOLINT

  /**
   * @brief Reliquenshes ownership & opts out of RAII, giving you the raw
   * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
   */
  static auto unwrap(Box box) -> MutPtr {
    const auto ptr = box.raw_ptr();
    box.obj = nullptr;
    return ptr;
  }

  Box() = delete;

  Box(const Box &) = delete;

  Box(Box &&from) noexcept : obj(std::exchange(from.obj, nullptr)) {}

  // ReSharper disable once CppNonExplicitConvertingConstructor
  template<typename Derived>
    requires std::is_base_of_v<T, Derived>
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

  auto operator=(Box &&rhs) noexcept -> Box & {
    if (&rhs == this) return *this;

    drop();
    obj = std::exchange(rhs.obj, nullptr);

    return *this;
  }

  template<typename Derived>
    requires std::is_base_of_v<T, Derived> and (not std::is_same_v<T, Derived>)
  auto operator=(Box<Derived> &&rhs) noexcept -> Box & {
    if (&rhs == this) return *this;

    drop();
    obj = static_cast<T *>(Box<Derived>::unwrap(std::forward<Box<Derived>>(rhs)));

    return *this;
  }

  [[nodiscard]] auto operator->() -> MutPtr { return as_ptr(); }

  [[nodiscard]] auto operator->() const -> ConstPtr { return as_ptr(); }

  [[nodiscard]] auto operator*() -> Contained & { return *as_ptr(); }

  auto operator*() const -> const Contained & { return *as_ptr(); }

  friend auto operator<<(std::ostream &os, const Box &rhs) -> std::ostream & { return os << *rhs; }

  /**
   * @brief Gets the underlying raw pointer for this box.
   */
  [[nodiscard]] auto as_ptr() -> MutPtr { return raw_ptr(); }

  /**
   * @brief Gets the underlying raw pointer for this box.
   */
  [[nodiscard]] auto as_ptr() const -> ConstPtr { return raw_ptr(); }

  template<std::derived_from<Contained> Derived>
  [[nodiscard]] auto downcast() const -> Option<Ref<Derived>> {
    return crab::ref::from_ptr(dynamic_cast<const T *>(raw_ptr()));
  }

  template<std::derived_from<Contained> Derived>
  [[nodiscard]] auto downcast() -> Option<RefMut<Derived>> {
    return crab::ref::from_ptr(dynamic_cast<T *>(raw_ptr()));
  }

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

  /**
   * @brief Reliquenshes ownership & opts out of RAII, giving you the raw
   * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
   */
  template<typename T>
  static auto release(Box<T> box) -> typename Box<T>::MutPtr {
    return Box<T>::unwrap(std::move(box));
  }
} // namespace crab
