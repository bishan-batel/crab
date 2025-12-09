#pragma once
#include <concepts>
#include <iostream>

#include <type_traits>
#include <utility>

#include <crab/casts.hpp>
#include <crab/debug.hpp>
#include <crab/preamble.hpp>
#include <crab/ref.hpp>
#include <crab/type_traits.hpp>
#include "crab/option.hpp"

// NOLINTBEGIN(*explicit*)

/**
 * @brief Owned Pointer (RAII) to an instance of T on the heap, this is an owned smart pointer type with no interior
 * mutability and and who is always non-null for a given valid value of Box<T>.
 *
 * A moved-from box does not leave it in a "valid but unspecified state" like other "well-behaved" types in the STL
 * (circa 2024), but leaves it in a state where it is partially-formed (only safe to reassign or destroy).
 *
 * If you are using this as a field of a class and don't want your class to be in this 'only safe to reassign or
 * destroy' state once moved, you must take on the weaker invariant and use Option<Box<T>> to explicitly allowed Box<T>
 * to be none.
 *
 * This is a replacement for std::unique_ptr, with two key differences:
 *
 * - Prevents Interior Mutability, being passed a const Box<T>& means dealing
 * with a const T&
 *
 * - A Box<T> has the invariant of always being non null, if it is then you
 * messed up with move semantics.
 */
template<typename T>
class Box;

namespace crab::option {

  template<typename T>
  struct BoxStorage;

  template<typename T>
  struct Storage;

  /**
   * @brief Storage type
   */
  template<typename T>
  struct Storage<Box<T>> final {
    using type = BoxStorage<T>;
  };
}

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
class Box {
  T* obj;

  static_assert(
    not crab::ty::is_const<T>,
    "Box<T> does not support const undirected types, switch "
    "Box<const T> into "
    "Box<T>"
  );

  // SizeType size;
  CRAB_INLINE_CONSTEXPR explicit Box(T* const from): obj(from) {}

  /**
   * Deletes the inner content, leaving this value partially formed
   */
  CRAB_INLINE_CONSTEXPR auto drop() -> void {
    if constexpr (crab::ty::array<T>) {
      delete[] obj;
    } else {
      delete obj;
    }
  }

public:

  friend struct crab::option::BoxStorage<T>;

  /**
   * @brief Wraps pointer with RAII
   *
   * This function has no way of knowing if the pointer passed is actually on
   * the heap or if something else has ownership, therefor 'unchecked' and it is
   * the responsibility of the caller to make sure.
   */
  CRAB_PURE_INLINE_CONSTEXPR static auto wrap_unchecked(T* const ref) -> Box {
    return Box(ref);
  };

  /**
   * Gives up ownership & opts out of RAII, giving you the raw
   * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
   */
  CRAB_PURE_INLINE_CONSTEXPR static auto unwrap(Box box, const SourceLocation loc = SourceLocation::current()) -> T* {
    return std::exchange(box.raw_ptr(loc), nullptr);
  }

  Box() = delete;

  Box(const Box&) = delete;

  CRAB_INLINE_CONSTEXPR Box(Box&& from) noexcept: obj{std::exchange(from.obj, nullptr)} {}

  template<std::derived_from<T> Derived>
  CRAB_INLINE_CONSTEXPR Box(Box<Derived> from, const SourceLocation loc = SourceLocation::current()):
      Box{Box<Derived>::unwrap(std::move(from))} {
    debug_assert_transparent(obj != nullptr, loc, "Invalid Box, moved from invalid box.");
  }

  /**
   * Destructor of Box<T>
   */
  CRAB_INLINE_CONSTEXPR ~Box() {
    drop();
  }

  /**
   * Implicit conversion from `Box<T>` -> `T&`
   */
  CRAB_INLINE_CONSTEXPR operator T&() {
    return *raw_ptr();
  }

  /**
   * Implicit conversion from `const Box<T>` -> `const T&`
   */
  CRAB_INLINE_CONSTEXPR operator const T&() const {
    return *raw_ptr();
  }

  template<std::derived_from<T> Base>
  CRAB_INLINE_CONSTEXPR operator Base&() {
    return *raw_ptr();
  }

  template<std::derived_from<T> Base>
  CRAB_INLINE_CONSTEXPR operator const Base&() const {
    return *raw_ptr();
  }

  CRAB_INLINE_CONSTEXPR operator Ref<T>() const {
    return *raw_ptr();
  }

  CRAB_INLINE_CONSTEXPR operator RefMut<T>() {
    return *raw_ptr();
  }

  template<std::derived_from<T> Base>
  CRAB_INLINE_CONSTEXPR operator Ref<Base>() const {
    return *raw_ptr();
  }

  template<std::derived_from<T> Base>
  CRAB_INLINE_CONSTEXPR operator RefMut<Base>() {
    return *raw_ptr();
  }

  auto operator=(const Box&) -> void = delete;

  CRAB_INLINE_CONSTEXPR auto operator=(Box&& rhs) noexcept -> Box& {
    if (rhs.obj == obj) {
      return *this;
    }

    drop();
    obj = std::exchange(rhs.obj, nullptr);

    return *this;
  }

  template<std::derived_from<T> Derived>
  CRAB_INLINE_CONSTEXPR auto operator=(Box<Derived>&& rhs) noexcept -> Box& {
    if (obj == static_cast<T*>(rhs.as_ptr())) {
      return *this;
    }

    drop();
    obj = static_cast<T*>(Box<Derived>::unwrap(std::forward<Box<Derived>>(rhs)));

    return *this;
  }

  CRAB_PURE_INLINE_CONSTEXPR auto operator->() -> T* {
    return as_ptr();
  }

  CRAB_PURE_INLINE_CONSTEXPR auto operator->() const -> const T* {
    return as_ptr();
  }

  CRAB_PURE_INLINE_CONSTEXPR auto operator*() -> T& {
    return *as_ptr();
  }

  CRAB_PURE_INLINE_CONSTEXPR auto operator*() const -> const T& {
    return *as_ptr();
  }

  CRAB_INLINE_CONSTEXPR friend auto operator<<(std::ostream& os, const Box& rhs) -> std::ostream& {
    return os << *rhs;
  }

  /**
   * @brief Gets the underlying raw pointer for this box.
   */
  CRAB_PURE_INLINE_CONSTEXPR auto as_ptr(const SourceLocation loc = SourceLocation::current()) -> T* {
    return raw_ptr(loc);
  }

  /**
   * @brief Gets the underlying raw pointer for this box.
   */
  CRAB_PURE_INLINE_CONSTEXPR auto as_ptr(const SourceLocation loc = SourceLocation::current()) const -> const T* {
    return raw_ptr(loc);
  }

  /**
   * @brief Attempts to downcast the pointer
   */
  template<std::derived_from<T> Derived>
  CRAB_PURE_INLINE_CONSTEXPR auto downcast(const SourceLocation loc = SourceLocation::current()) const
    -> Option<const Derived&> {
    return crab::ref::from_ptr(dynamic_cast<const Derived*>(raw_ptr(loc)));
  }

  /**
   * @brief Attempts to downcast the pointer
   */
  template<std::derived_from<T> Derived>
  CRAB_PURE_INLINE_CONSTEXPR auto downcast(const SourceLocation loc = SourceLocation::current()) -> Option<Derived&> {
    return crab::ref::from_ptr(dynamic_cast<Derived*>(raw_ptr(loc)));
  }

  /**
   * @brief Upcasts to a base type
   */
  template<typename Base>
  requires std::derived_from<T, Base>
  CRAB_PURE_INLINE_CONSTEXPR auto upcast() const -> const Base& {
    return *raw_ptr();
  }

  /**
   * @brief Upcasts to a base type
   */
  template<typename Base>
  requires std::derived_from<T, Base>
  CRAB_PURE_INLINE_CONSTEXPR auto upcast() -> Base& {
    return *raw_ptr();
  }

  /**
   * @brief Attempts to downcast this box to another,
   * This function will either transfer ownership to a returned Some value, or
   * this data will be deleted entirely, invalidating this object & returning
   * crab::none
   */
  template<std::derived_from<T> Derived>
  CRAB_PURE_INLINE_CONSTEXPR auto downcast_lossy() && -> Option<Box<Derived>> {
    auto* ptr = dynamic_cast<Derived*>(raw_ptr());

    if (ptr == nullptr) {
      drop();
      return {};
    }

    obj = nullptr;
    return Box<Derived>::wrap_unchecked(std::exchange(ptr, nullptr));
  }

private:

  CRAB_PURE_INLINE_CONSTEXPR auto raw_ptr(const SourceLocation loc = SourceLocation::current()) -> T*& {

    debug_assert_transparent(obj != nullptr, loc, "Invalid Use of Moved Box<T>.");

    return reinterpret_cast<T*&>(obj);
  }

  CRAB_PURE_INLINE_CONSTEXPR auto raw_ptr(const SourceLocation loc = SourceLocation::current()) const -> const T* {
    debug_assert_transparent(obj != nullptr, loc, "Invalid Use of Moved Box<T>.");
    return reinterpret_cast<const T*>(obj);
  }
};

namespace crab::option {

  template<typename T>
  struct BoxStorage final {
    using Box = ::Box<T>;

    CRAB_INLINE_CONSTEXPR explicit BoxStorage(Box value): inner{std::move(value)} {}

    CRAB_INLINE_CONSTEXPR explicit BoxStorage(const None& = crab::none): inner{nullptr} {}

    CRAB_INLINE_CONSTEXPR auto operator=(Box&& value) -> BoxStorage& {
      debug_assert(value.obj != nullptr, "Option<Box<T>>, BoxStorage::operator= called with an invalid box");
      inner = std::move(value);
      return *this;
    }

    CRAB_INLINE_CONSTEXPR auto operator=(const None&) -> BoxStorage& {
      crab::discard(Box{std::move(inner)});
      return *this;
    }

    CRAB_PURE_INLINE_CONSTEXPR auto value() const& -> const Box& {
      return inner;
    }

    CRAB_PURE_INLINE_CONSTEXPR auto value() & -> Box& {
      return inner;
    }

    CRAB_PURE_INLINE_CONSTEXPR auto value() && -> Box {
      return std::move(inner);
    }

    CRAB_PURE_INLINE_CONSTEXPR auto in_use() const -> bool {
      return inner.obj != nullptr;
    }

  private:

    Box inner;
  };
}

namespace crab {
  /**
   * @brief Makes a new instance of type T on the heap with given args
   */
  template<crab::complete_type T, typename... Args>
  requires std::constructible_from<T, Args...>
  CRAB_PURE_INLINE_CONSTEXPR static auto make_box(Args&&... args) -> Box<T> {
    return Box<T>::wrap_unchecked(new T{std::forward<Args>(args)...});
  }

  /**
   * @brief Makes a new instance of type T on the heap with given args
   */
  template<typename T, typename V>
  requires std::convertible_to<T, V> and (std::integral<T> or std::floating_point<T>)
  CRAB_PURE_INLINE_CONSTEXPR static auto make_box(V&& from) -> Box<T> {
    return Box<T>::wrap_unchecked(new T{static_cast<T>(from)});
  }
} // namespace crab

// NOLINTEND(*explicit*)
