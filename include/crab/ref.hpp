#pragma once

#include <functional>

#include <crab/debug.hpp>
#include <crab/type_traits.hpp>
#include <source_location>
#include "crab/core.hpp"

// NOLINTBEGIN(*explicit*)

/**
 * Reference to some type T that is always NON NULL, use in place of 'const T&'
 * when applicable (for ex. inside template parameters)
 */
template<crab::ref::is_valid_type T>
class Ref final {
  CRAB_INLINE_CONSTEXPR explicit Ref(const T* const pointer, SourceLocation loc = SourceLocation::current()):
      pointer(pointer) {
    debug_assert_transparent(pointer, loc, "Invalid State: Cannot create a NULL Ref object");
  }

public:

  CRAB_PURE_INLINE_CONSTEXPR static Ref from_unchecked(const T* const pointer) {
    return Ref(pointer);
  }

  CRAB_INLINE_CONSTEXPR Ref(const T& ref): Ref(&ref) {}

  /**
   * You cannot construct a reference to a xalue
   */
  Ref(T&& ref) = delete;

  /**
   * You cannot construct a reference to an rvalue
   */
  Ref(const T&& ref) = delete;

  CRAB_PURE_INLINE_CONSTEXPR operator const T&() const {
    return get_ref();
  };

  CRAB_PURE_INLINE_CONSTEXPR operator const T*() const {
    return as_ptr();
  };

  CRAB_PURE_INLINE_CONSTEXPR const T& operator*() const {
    return get_ref();
  }

  CRAB_PURE_INLINE_CONSTEXPR const T* operator->() const {
    return as_ptr();
  }

  /**
   * Gets underlying pointer, this pointer is always non null
   */
  CRAB_PURE_INLINE_CONSTEXPR const T* as_ptr() const {
    return pointer;
  }

  /**
   * Gets a C++ reference to underlying data
   */
  CRAB_PURE_INLINE_CONSTEXPR const T& get_ref() const {
    return *pointer;
  }

  friend constexpr auto operator<<(std::ostream& os, const Ref& val) -> std::ostream& {
    if constexpr (requires(const T& val) { os << val; }) {
      return os << *val;
    } else {
      return os << val.as_ptr();
    }
  }

private:

  const T* pointer;
};

template<crab::ref::is_valid_type T>
class RefMut final {
  constexpr explicit RefMut(T* const pointer, SourceLocation loc = SourceLocation::current()): pointer(pointer) {
    debug_assert_transparent(pointer, loc, "Invalid State: Cannot create a NULL RefMut object");
  }

public:

  constexpr RefMut(T& ref): RefMut(&ref) {}

  CRAB_PURE_INLINE_CONSTEXPR static RefMut from_unchecked(T* const pointer) {
    return RefMut(pointer);
  }

  CRAB_PURE_INLINE_CONSTEXPR operator T&() const {
    return get_mut_ref();
  };

  CRAB_PURE_INLINE_CONSTEXPR operator T*() const {
    return as_ptr();
  };

  CRAB_PURE_INLINE_CONSTEXPR operator Ref<T>() const {
    return as_ref();
  };

  CRAB_PURE_INLINE_CONSTEXPR T& operator*() const {
    return get_mut_ref();
  }

  CRAB_PURE_INLINE_CONSTEXPR T* operator->() const {
    return as_ptr();
  }

  /**
   * Gets underlying pointer, this pointer is always non null
   *
   */
  CRAB_PURE_INLINE_CONSTEXPR T* as_ptr() const {
    return pointer;
  }

  /**
   * Gets a C++ reference to underlying data
   */
  CRAB_PURE_INLINE_CONSTEXPR T& get_mut_ref() const {
    return *pointer;
  }

  /**
   * Gets a C++ reference to underlying data
   */
  CRAB_PURE_INLINE_CONSTEXPR const T& get_ref() const {
    return *pointer;
  }

  /**
   * Converts RefMut into a immutable reference
   */
  CRAB_PURE_INLINE_CONSTEXPR Ref<T> as_ref() const {
    return Ref<T>(get_mut_ref());
  }

  friend constexpr auto operator<<(std::ostream& os, const RefMut& val) -> std::ostream& {
    if constexpr (requires(const T& val) { os << val; }) {
      return os << *val;
    } else {
      return os << val.as_ptr();
    }
  }

private:

  T* pointer;
};

/**
 * @brief Hash Implementation for Ref<T> is identical for std::hash<T>
 */
template<typename T>
struct std::hash<Ref<T>> {
  CRAB_PURE_INLINE_CONSTEXPR auto operator()(const Ref<T>& mut) const -> usize {
    return std::hash<const T*>(mut.as_ptr());
  };
};

template<typename T>
struct std::hash<RefMut<T>> : std::hash<Ref<T>> {};

// NOLINTEND(*explicit*)
