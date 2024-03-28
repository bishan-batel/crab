#pragma once
#include <cassert>
#include <preamble.hpp>
#include <type_traits>

/**
 * Reference to some type T that is always NON NULL, use in place of 'const T&'
 * when applicable (for ex. inside template parameters)
 */
template <typename T>
  requires(not std::is_const_v<T> and not std::is_reference_v<T>)
class Ref final {
  explicit constexpr inline Ref(const T *pointer) : pointer(pointer) {
    assert(pointer && "Invalid State: Cannot create a NULL Ref object");
  }

public:
  [[nodiscard]] constexpr inline static Ref from_unchecked(T *pointer) {
    return Ref(pointer);
  }

  constexpr inline Ref(const T &ref) : Ref(&ref) {}

  [[nodiscard]] constexpr inline operator const T &() const {
    return get_ref();
  };
  [[nodiscard]] constexpr inline operator const T *() const {
    return get_raw_pointer();
  };

  [[nodiscard]] constexpr inline const T &operator*() const {
    return get_ref();
  }

  [[nodiscard]] constexpr inline const T *operator->() const {
    return get_raw_pointer();
  }

  /**
   * Gets underlying pointer, this pointer is always non null
   *
   */
  [[nodiscard]] constexpr inline const T *get_raw_pointer() const {
    return pointer;
  }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] constexpr inline const T &get_ref() const { return *pointer; }

private:
  const T *pointer;
};

template <typename T>
  requires(not std::is_const_v<T> and not std::is_reference_v<T>)
class RefMut final {
  constexpr inline explicit RefMut(T *pointer) : pointer(pointer) {
    assert(pointer && "Invalid State: Cannot create a NULL RefMut object");
  }

public:
  constexpr inline RefMut(T &ref) : RefMut(&ref) {}

  [[nodiscard]] constexpr inline static RefMut from_unchecked(T *pointer) {
    return RefMut(pointer);
  }

  [[nodiscard]] constexpr inline operator T &() const { return get_mut_ref(); };

  [[nodiscard]] constexpr inline operator T *() const {
    return get_raw_pointer();
  };

  [[nodiscard]] constexpr inline operator Ref<T>() const { return as_ref(); };

  [[nodiscard]] constexpr inline T &operator*() const { return get_mut_ref(); }

  [[nodiscard]] constexpr inline T *operator->() const {
    return get_raw_pointer();
  }

  /**
   * Gets underlying pointer, this pointer is always non null
   *
   */
  [[nodiscard]] constexpr inline T *get_raw_pointer() const { return pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] constexpr inline T &get_mut_ref() const { return *pointer; }

  [[nodiscard]] constexpr inline Ref<T> as_ref() const {
    return Ref<T>(get_mut_ref());
  }

private:
  T *pointer;
};

template <typename T> class Option;

namespace ref {
template <typename T> [[nodiscard]] Ref<T> constexpr inline of(const T &from) {
  return Ref(from);
}

template <typename T> [[nodiscard]] constexpr inline RefMut<T> of(T &from) {
  return RefMut(from);
}

template <typename T>
[[nodiscard]] constexpr inline Ref<T> from_unchecked(const T *const from) {
  return Ref<T>::from_unchecked(from);
}

template <typename T>
[[nodiscard]] constexpr inline RefMut<T> from_unchecked(T *const from) {
  return RefMut<T>::from_unchecked(from);
}

} // namespace ref

#include "option.hpp"

namespace ref {
template <typename T>
[[nodiscard]] constexpr inline Option<RefMut<T>> from(T *const from) {
  return from ? Option(RefMut(from)) : opt::none;
}

template <typename T>
[[nodiscard]] constexpr inline Option<Ref<T>> from(const T *const from) {
  return from ? Option(Ref(from)) : opt::none;
}

} // namespace ref
