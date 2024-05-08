// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppNonExplicitConversionOperator
#pragma once
#include "crab/type_traits.hpp"
#include "crab/debug.hpp"

/**
 * Reference to some type T that is always NON NULL, use in place of 'const T&'
 * when applicable (for ex. inside template parameters)
 */
template<typename T> requires crab::ref::is_valid_type<T>
class Ref final {
  explicit  Ref(const T *const pointer)
    : pointer(pointer) {
    debug_assert(pointer, "Invalid State: Cannot create a NULL Ref object");
  }

public:
  [[nodiscard]] static Ref from_unchecked(const T *const pointer) {
    return Ref(pointer);
  }

   Ref(const T &ref) : Ref(&ref) {}

  [[nodiscard]]  operator const T&() const {
    return get_ref();
  };

  [[nodiscard]]  operator const T*() const {
    return as_ptr();
  };

  [[nodiscard]]  const T& operator*() const {
    return get_ref();
  }

  [[nodiscard]]  const T *operator->() const {
    return as_ptr();
  }

  /**
   * Gets underlying pointer, this pointer is always non null
   *
   */
  [[nodiscard]]  const T *as_ptr() const {
    return pointer;
  }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]]  const T& get_ref() const { return *pointer; }

private:
  const T *pointer;
};

template<typename T> requires crab::ref::is_valid_type<T>
class RefMut final {
   explicit RefMut(T *const pointer)
    : pointer(pointer) {
    debug_assert(pointer, "Invalid State: Cannot create a NULL RefMut object");
  }

public:
   RefMut(T &ref) : RefMut(&ref) {}

  [[nodiscard]] static RefMut from_unchecked(T *const pointer) {
    return RefMut(pointer);
  }

  [[nodiscard]]  operator T&() const { return get_mut_ref(); };

  [[nodiscard]]  operator T*() const {
    return as_ptr();
  };

  [[nodiscard]]  operator Ref<T>() const { return as_ref(); };

  [[nodiscard]]  T& operator*() const { return get_mut_ref(); }

  [[nodiscard]]  T *operator->() const {
    return as_ptr();
  }

  /**
   * Gets underlying pointer, this pointer is always non null
   *
   */
  [[nodiscard]]  T *as_ptr() const { return pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]]  T& get_mut_ref() const { return *pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]]  const T& get_ref() const { return *pointer; }

  /**
   * Converts RefMut into a immutable reference
   */
  [[nodiscard]]  Ref<T> as_ref() const {
    return Ref<T>(get_mut_ref());
  }

private:
  T *pointer;
};

template<typename T>
class Option;

namespace crab::ref {
  template<typename T>
  [[nodiscard]]  Ref<T> from_ptr_unchecked(const T *const from) {
    return Ref<T>::from_unchecked(from);
  }

  template<typename T>
  [[nodiscard]]  RefMut<T> from_ptr_unchecked(T *const from) {
    return RefMut<T>::from_unchecked(from);
  }
}

#include "option.hpp"

namespace crab::ref {
  /**
   *
   */
  template<typename T>
  [[nodiscard]]  Option<RefMut<T>> from_ptr(T *const from) {
    if (from) {
      return some(from_ptr_unchecked(from));
    }
    return none;
  }

  template<typename T>
  [[nodiscard]]  Option<Ref<T>> from_ptr(const T *const from) {
    if (from) {
      return some(from_ptr_unchecked(from));
    }
    return none;
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base> requires std::is_base_of_v<Base, Derived>
  Option<Ref<Derived>> cast(const Base &from) {
    return from_ptr(dynamic_cast<const Derived*>(&from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base> requires std::is_base_of_v<Base, Derived>
  Option<RefMut<Derived>> cast(Base &from) {
    return from_ptr(dynamic_cast<Derived*>(&from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base> requires std::is_base_of_v<Base, Derived>
  Option<Ref<Derived>> cast(const Base *from) {
    return from_ptr(dynamic_cast<const Derived*>(from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base> requires std::is_base_of_v<Base, Derived>
  Option<RefMut<Derived>> cast(Base *from) {
    return from_ptr(dynamic_cast<Derived*>(from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base> requires std::is_base_of_v<Base, Derived>
   Option<Ref<Derived>> cast(Ref<Base> from) {
    return cast(from.get_ref());
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base> requires std::is_base_of_v<Base, Derived>
   Option<RefMut<Derived>> cast(RefMut<Base> from) {
    return cast(from.get_ref());
  }
}
