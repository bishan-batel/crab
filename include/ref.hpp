// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppNonExplicitConversionOperator
#pragma once
#include <crab_type_traits.hpp>
#include "preamble.hpp"
#include "debug.hpp"

/**
 * Reference to some type T that is always NON NULL, use in place of 'const T&'
 * when applicable (for ex. inside template parameters)
 */
template<typename T> requires crab::ref::is_valid_type<T>
class Ref final {
  explicit __always_inline Ref(const T *pointer)
    : pointer(pointer) {
    debug_assert(pointer, "Invalid State: Cannot create a NULL Ref object");
  }

public:
  [[nodiscard]] __always_inline static Ref from_unchecked(T *pointer) {
    return Ref(pointer);
  }

  __always_inline Ref(const T &ref) : Ref(&ref) {}

  [[nodiscard]] __always_inline operator const T &() const {
    return get_ref();
  };

  [[nodiscard]] __always_inline operator const T *() const {
    return as_ptr();
  };

  [[nodiscard]] __always_inline const T &operator*() const {
    return get_ref();
  }

  [[nodiscard]] __always_inline const T *operator->() const {
    return as_ptr();
  }

  /**
   * Gets underlying pointer, this pointer is always non null
   *
   */
  [[nodiscard]] __always_inline const T *as_ptr() const {
    return pointer;
  }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] __always_inline const T &get_ref() const { return *pointer; }

private:
  const T *pointer;
};

template<typename T> requires crab::ref::is_valid_type<T>
class RefMut final {
  __always_inline explicit RefMut(T *pointer)
    : pointer(pointer) {
    debug_assert(pointer, "Invalid State: Cannot create a NULL RefMut object");
  }

public:
  __always_inline RefMut(T &ref) : RefMut(&ref) {}

  [[nodiscard]] __always_inline static RefMut from_unchecked(T *pointer) {
    return RefMut(pointer);
  }

  [[nodiscard]] __always_inline operator T &() const { return get_mut_ref(); };

  [[nodiscard]] __always_inline operator T *() const {
    return as_ptr();
  };

  [[nodiscard]] __always_inline operator Ref<T>() const { return as_ref(); };

  [[nodiscard]] __always_inline T &operator*() const { return get_mut_ref(); }

  [[nodiscard]] __always_inline T *operator->() const {
    return as_ptr();
  }

  /**
   * Gets underlying pointer, this pointer is always non null
   *
   */
  [[nodiscard]] __always_inline T *as_ptr() const { return pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] __always_inline T &get_mut_ref() const { return *pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] __always_inline const T &get_ref() const { return *pointer; }

  /**
   * Converts RefMut into a immutable reference
   */
  [[nodiscard]] __always_inline Ref<T> as_ref() const {
    return Ref<T>(get_mut_ref());
  }

private:
  T *pointer;
};

template<typename T>
class Option;

namespace crab::ref {
  template<typename T>
  [[nodiscard]] __always_inline Ref<T> from_ptr_unchecked(const T *const from) {
    return Ref<T>::from_unchecked(from);
  }

  template<typename T>
  [[nodiscard]] __always_inline RefMut<T> from_ptr_unchecked(T *const from) {
    return RefMut<T>::from_unchecked(from);
  }
}

#include "option.hpp"

namespace crab::ref {
  /**
   *
   */
  template<typename T>
  [[nodiscard]] Option<RefMut<T> > from_ptr(T *const from) {
    if (from) {
      return some(RefMut(from));
    }
    return none;
  }

  template<typename T>
  [[nodiscard]] Option<Ref<T> > from_ptr(const T *const from) {
    if (from) {
      return some(Ref(from));
    }
    return none;
  }

  template<typename Derived, typename Base> requires std::is_base_of_v<Base, Derived>
  Option<Ref<Derived> > cast(const Base &from) {
    return from_ptr(dynamic_cast<const Derived *>(&from));
  }

  template<typename Derived, typename Base> requires std::is_base_of_v<Base, Derived>
  Option<RefMut<Derived> > cast(Base &from) {
    return from_ptr(dynamic_cast<Derived *>(&from));
  }

  template<typename Derived, typename Base> requires std::is_base_of_v<Base, Derived>
  Option<Ref<Derived> > cast(Ref<Base> from) {
    return cast(from.get_ref());
  }

  template<typename Derived, typename Base> requires std::is_base_of_v<Base, Derived>
  Option<RefMut<Derived> > cast(RefMut<Base> from) {
    return cast(from.get_ref());
  }
}
