#pragma once
#include <functional>
#include <type_traits>
#include "crab/debug.hpp"
#include "crab/type_traits.hpp"

/**
 * Reference to some type T that is always NON NULL, use in place of 'const T&'
 * when applicable (for ex. inside template parameters)
 */
template<typename T>
  requires crab::ref::is_valid_type<T>
class Ref final {
  constexpr explicit Ref(const T *const pointer) : pointer(pointer) {
    debug_assert(pointer, "Invalid State: Cannot create a NULL Ref object");
  }

public:
  [[nodiscard]] constexpr static Ref from_unchecked(const T *const pointer) { return Ref(pointer); }

  constexpr Ref(const T &ref) : // NOLINT(*-explicit-constructor)
      Ref(&ref) {}

  [[nodiscard]] constexpr operator const T &() const { // NOLINT(*-explicit-constructor)
    return get_ref();
  };

  [[nodiscard]] constexpr operator const T *() const { // NOLINT(*-explicit-constructor)
    return as_ptr();
  };

  [[nodiscard]] constexpr const T &operator*() const { return get_ref(); }

  [[nodiscard]] constexpr const T *operator->() const { return as_ptr(); }

  /**
   * Gets underlying pointer, this pointer is always non null
   */
  [[nodiscard]] constexpr const T *as_ptr() const { return pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] constexpr const T &get_ref() const { return *pointer; }

private:
  const T *pointer;
};

template<typename T>
  requires crab::ref::is_valid_type<T>
class RefMut final {
  constexpr explicit RefMut(T *const pointer) : pointer(pointer) {
    debug_assert(pointer, "Invalid State: Cannot create a NULL RefMut object");
  }

public:
  constexpr RefMut(T &ref) : RefMut(&ref) {} // NOLINT(*-explicit-constructor)

  [[nodiscard]] constexpr static RefMut from_unchecked(T *const pointer) { return RefMut(pointer); }

  [[nodiscard]] constexpr operator T &() const { return get_mut_ref(); }; // NOLINT(*-explicit-constructor)

  [[nodiscard]] constexpr operator T *() const { return as_ptr(); }; // NOLINT(*-explicit-constructor)

  [[nodiscard]] constexpr operator Ref<T>() const { return as_ref(); }; // NOLINT(*-explicit-constructor)

  [[nodiscard]] constexpr T &operator*() const { return get_mut_ref(); }

  [[nodiscard]] constexpr T *operator->() const { return as_ptr(); }

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
  [[nodiscard]] constexpr Ref<T> as_ref() const { return Ref<T>(get_mut_ref()); }

private:
  T *pointer;
};

template<typename T>
struct std::hash<RefMut<T>> { // NOLINT
  auto operator()(const RefMut<T> &mut) const -> usize { return std::bit_cast<usize>(mut.as_ptr()); };
};

template<typename T>
struct std::hash<Ref<T>> { // NOLINT
  auto operator()(const Ref<T> &mut) const -> usize { return std::bit_cast<usize>(mut.as_ptr()); };
};

template<typename T>
class Option;

namespace crab::ref {
  template<typename T>
  [[nodiscard]] constexpr Ref<T> from_ptr_unchecked(const T *const from) {
    return Ref<T>::from_unchecked(from);
  }

  template<typename T>
  [[nodiscard]] constexpr RefMut<T> from_ptr_unchecked(T *const from) {
    return RefMut<T>::from_unchecked(from);
  }
} // namespace crab::ref

#include "option.hpp"

namespace crab::ref {
  /**
   *
   */
  template<typename T>
  [[nodiscard]] constexpr Option<RefMut<T>> from_ptr(T *const from) {
    if (from) {
      return some(from_ptr_unchecked(from));
    }
    return crab::none;
  }

  template<typename T>
  [[nodiscard]] constexpr Option<Ref<T>> from_ptr(const T *const from) {
    if (from) {
      return some(from_ptr_unchecked(from));
    }
    return none;
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base>
    requires std::is_base_of_v<Base, Derived>
  constexpr auto cast(const Base &from) -> Option<Ref<Derived>> {
    return from_ptr(dynamic_cast<const Derived *>(&from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base>
    requires std::is_base_of_v<Base, Derived>
  constexpr auto cast(Base &from) -> Option<RefMut<Derived>> {
    return from_ptr(dynamic_cast<Derived *>(&from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base>
    requires std::is_base_of_v<Base, Derived>
  constexpr auto cast(const Base *from) -> Option<Ref<Derived>> {
    return from_ptr(dynamic_cast<const Derived *>(from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base>
    requires std::is_base_of_v<Base, Derived>
  constexpr auto cast(Base *from) -> Option<RefMut<Derived>> {
    return from_ptr(dynamic_cast<Derived *>(from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base>
    requires std::is_base_of_v<Base, Derived>
  constexpr Option<Ref<Derived>> cast(Ref<Base> from) {
    return cast(from.get_ref());
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base>
    requires std::is_base_of_v<Base, Derived>
  constexpr Option<RefMut<Derived>> cast(RefMut<Base> from) {
    return cast(from.get_ref());
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base>
    requires std::is_base_of_v<Base, Derived>
  constexpr Option<Ref<Derived>> cast(Option<Ref<Base>> from) {
    return from.flat_map([](Ref<Base> base) -> Option<Ref<Derived>> { return cast<Derived, Base>(base); });
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<typename Derived, typename Base>
    requires std::is_base_of_v<Base, Derived>
  constexpr Option<Ref<Derived>> cast(Option<RefMut<Base>> from) {
    return from.flat_map([](RefMut<Base> base) -> Option<RefMut<Derived>> { return cast<Derived, Base>(base); });
  }
} // namespace crab::ref
