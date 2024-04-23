// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppNonExplicitConversionOperator
#pragma once
#include <type_traits>
#include "preamble.hpp"
#include "debug.hpp"

/**
 * Reference to some type T that is always NON NULL, use in place of 'const T&'
 * when applicable (for ex. inside template parameters)
 */
template<typename T>
  requires(not std::is_const_v<T> and not std::is_reference_v<T>)
class Ref final {
  explicit constexpr Ref(const T *pointer)
    : pointer(pointer) {
    debug_assert(pointer, "Invalid State: Cannot create a NULL Ref object");
  }

public:
  [[nodiscard]] constexpr static Ref from_unchecked(T *pointer) {
    return Ref(pointer);
  }

  constexpr Ref(const T &ref) : Ref(&ref) {}

  [[nodiscard]] constexpr operator const T &() const {
    return get_ref();
  };

  [[nodiscard]] constexpr operator const T *() const {
    return as_ptr();
  };

  [[nodiscard]] constexpr const T &operator*() const {
    return get_ref();
  }

  [[nodiscard]] constexpr const T *operator->() const {
    return as_ptr();
  }

  /**
   * Gets underlying pointer, this pointer is always non null
   *
   */
  [[nodiscard]] const T *as_ptr() const {
    return pointer;
  }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] const T &get_ref() const { return *pointer; }

private:
  const T *pointer;
};

template<typename T>
  requires(not std::is_const_v<T> and not std::is_reference_v<T>)
class RefMut final {
  constexpr explicit RefMut(T *pointer)
    : pointer(pointer) {
    debug_assert(pointer, "Invalid State: Cannot create a NULL RefMut object");
  }

public:
  constexpr RefMut(T &ref) : RefMut(&ref) {}

  [[nodiscard]] static RefMut from_unchecked(T *pointer) {
    return RefMut(pointer);
  }

  [[nodiscard]] operator T &() const { return get_mut_ref(); };

  [[nodiscard]] operator T *() const {
    return as_ptr();
  };

  [[nodiscard]] operator Ref<T>() const { return as_ref(); };

  [[nodiscard]] T &operator*() const { return get_mut_ref(); }

  [[nodiscard]] T *operator->() const {
    return as_ptr();
  }

  /**
   * Gets underlying pointer, this pointer is always non null
   *
   */
  [[nodiscard]] constexpr T *as_ptr() const { return pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] constexpr T &get_mut_ref() const { return *pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] constexpr const T &get_ref() const { return *pointer; }

  /**
   * Converts RefMut into a immutable reference
   */
  [[nodiscard]] constexpr Ref<T> as_ref() const {
    return Ref<T>(get_mut_ref());
  }

private:
  T *pointer;
};

template<typename T>
class Option;

namespace crab::ref {
  template<typename T>
  [[nodiscard]] Ref<T> from_ptr_unchecked(const T *const from) {
    return Ref<T>::from_unchecked(from);
  }

  template<typename T>
  [[nodiscard]] RefMut<T> from_ptr_unchecked(T *const from) {
    return RefMut<T>::from_unchecked(from);
  }
}

#include "option.hpp"

namespace crab::ref {
  /**
   *
   */
  template<typename T>
  [[nodiscard]] constexpr Option<RefMut<T> > from_ptr(T *const from) {
    if (from) {
      return some(RefMut(from));
    }
    return none;
  }

  template<typename T>
  [[nodiscard]] constexpr Option<Ref<T> > from_ptr(const T *const from) {
    if (from) {
      return some(Ref(from));
    }
    return none;
  }
}
