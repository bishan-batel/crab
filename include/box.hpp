#pragma once
#include <concepts>
#include <iostream>

#include <type_traits>
#include <utility>
#include "preamble.hpp"
#include "crab/type_traits.hpp"

#include "crab/debug.hpp"
#include "ref.hpp"

/**
 * @brief Owned Pointer (RAII) to an instance of T on the heap.
 *
 * This is a replacement for std::unique_ptr, with two key differences:
 *
 * - Prevents Interior Mutability, being passed a const Box<T>& means dealing
 * with a const T&
 * - A Box<T> has the invariant of always being non null, if it is then you
 * messed up with move semantics.
 */
template<typename T>
requires(not std::is_const_v<T>)
class Box {
  T* obj;

  // SizeType size;

  constexpr explicit Box(T* const from): obj(from) {} // NOLINT

  // ReSharper disable once CppMemberFunctionMayBeConst
  constexpr auto drop() -> void {
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
  static constexpr auto wrap_unchecked(T* const ref) -> Box {
    return Box(ref);
  }; // NOLINT

  /**
   * @brief Reliquenshes ownership & opts out of RAII, giving you the raw
   * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
   */
  [[nodiscard]] static constexpr auto unwrap(Box box) -> T* {
    return std::exchange(box.raw_ptr(), nullptr);
  }

  Box() = delete;

  Box(const Box&) = delete;

  Box(Box&& from) noexcept: obj(std::exchange(from.obj, nullptr)) {}

  template<std::derived_from<T> Derived>
  Box(Box<Derived>&& from) // NOLINT
      :
      Box{Box<Derived>::unwrap(std::forward<Box<Derived>>(from))} {
    debug_assert(obj != nullptr, "Invalid Box, moved from invalid box.");
  }

  explicit Box(T&& val): Box(new T(std::move(val))) {}

  ~Box() { drop(); }

  operator T&() { // NOLINT(*-explicit-constructor)
    return *raw_ptr();
  }

  operator const T&() const { // NOLINT(*-explicit-constructor)
    return *raw_ptr();
  }

  template<typename Base>
  requires std::derived_from<T, Base>
  operator Base&() { // NOLINT(*-explicit-constructor)
    return *raw_ptr();
  }

  template<typename Base>
  requires std::derived_from<T, Base>
  operator const Base&() const { // NOLINT(*-explicit-constructor)
    return *raw_ptr();
  }

  operator Ref<T>() const { // NOLINT(*-explicit-constructor)
    return *raw_ptr();
  }

  operator RefMut<T>() { // NOLINT(*-explicit-constructor)
    return *raw_ptr();
  }

  template<typename Base>
  requires std::derived_from<T, Base>
  operator Ref<Base>() const { // NOLINT(*-explicit-constructor)
    return *raw_ptr();
  }

  template<typename Base>
  requires std::derived_from<T, Base>
  operator RefMut<Base>() { // NOLINT(*-explicit-constructor)
    return *raw_ptr();
  }

  auto operator=(const Box&) -> void = delete;

  auto operator=(Box&& rhs) noexcept -> Box& {
    if (rhs.obj == obj) {
      return *this;
    }

    drop();
    obj = std::exchange(rhs.obj, nullptr);

    return *this;
  }

  template<std::derived_from<T> Derived>
  auto operator=(Box<Derived>&& rhs) noexcept -> Box& {
    if (obj == static_cast<T*>(rhs.as_ptr())) {
      return *this;
    }

    drop();
    obj =
      static_cast<T*>(Box<Derived>::unwrap(std::forward<Box<Derived>>(rhs)));

    return *this;
  }

  [[nodiscard]] auto operator->() -> T* { return as_ptr(); }

  [[nodiscard]] auto operator->() const -> const T* { return as_ptr(); }

  [[nodiscard]] auto operator*() -> T& { return *as_ptr(); }

  auto operator*() const -> const T& { return *as_ptr(); }

  friend auto operator<<(std::ostream& os, const Box& rhs) -> std::ostream& {
    return os << *rhs;
  }

  /**
   * @brief Gets the underlying raw pointer for this box.
   */
  [[nodiscard]] auto as_ptr() -> T* { return raw_ptr(); }

  /**
   * @brief Gets the underlying raw pointer for this box.
   */
  [[nodiscard]] auto as_ptr() const -> const T* { return raw_ptr(); }

  /**
   * @brief Attempts to downcast the pointer
   */
  template<std::derived_from<T> Derived>
  [[nodiscard]] auto downcast() const -> Option<Ref<Derived>> {
    return crab::ref::from_ptr(dynamic_cast<const Derived*>(raw_ptr()));
  }

  /**
   * @brief Attempts to downcast the pointer
   */
  template<std::derived_from<T> Derived>
  [[nodiscard]] auto downcast() -> Option<RefMut<Derived>> {
    return crab::ref::from_ptr(dynamic_cast<Derived*>(raw_ptr()));
  }

  /**
   * @brief Upcasts to a base type
   */
  template<typename Base>
  requires std::derived_from<T, Base>
  [[nodiscard]] auto upcast() const -> const Base& {
    return *raw_ptr();
  }

  /**
   * @brief Upcasts to a base type
   */
  template<typename Base>
  requires std::derived_from<T, Base>
  [[nodiscard]] auto upcast() -> Base& {
    return *raw_ptr();
  }

  /**
   * @brief Attempts to downcast this box to another,
   * This function will either transfer ownership to a returned Some value, or
   * this data will be deleted entirely, invalidating this object & returning
   * crab::none
   */
  template<std::derived_from<T> Derived>
  [[nodiscard]] auto downcast_lossy() && -> Option<Box<Derived>> {
    auto* ptr = dynamic_cast<Derived*>(raw_ptr());

    if (ptr == nullptr) {
      drop();
      return {};
    }

    return Box<Derived>{std::exchange(obj, nullptr)};
  }

private:

  [[nodiscard]] auto raw_ptr() -> T*& {
    debug_assert(obj != nullptr, "Invalid Use of Moved Box<T>.");
    return obj;
  }

  [[nodiscard]] auto raw_ptr() const -> const T* {
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
  [[nodiscard]] static auto make_box(Args&&... args) -> Box<T> {
    return Box<T>::wrap_unchecked(new T{std::forward<Args>(args)...});
  }

  /**
   * @brief Makes a new instance of type T on the heap with given args
   */
  template<typename T, typename V>
  requires std::is_convertible_v<T, V>
         and (std::is_integral_v<T> or std::is_floating_point_v<T>)
  [[nodiscard]] static auto make_box(V&& from) -> Box<T> {
    return Box<T>::wrap_unchecked(new T{static_cast<T>(from)});
  }

  /**
   * @brief Reliquenshes ownership & opts out of RAII, giving you the raw
   * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
   */
  template<typename T>
  [[nodiscard]] static auto release(Box<T> box) -> T* {
    return Box<T>::unwrap(std::move(box));
  }
} // namespace crab
