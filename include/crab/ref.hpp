#pragma once
#include <cstdint>
#include <functional>
#include <type_traits>

#include <crab/debug.hpp>
#include <crab/type_traits.hpp>

// NOLINGBEGIN(*explicit*)

/**
 * Reference to some type T that is always NON NULL, use in place of 'const T&'
 * when applicable (for ex. inside template parameters)
 */
template<crab::ref::is_valid_type T>
class Ref final {
  constexpr explicit Ref(const T* const pointer): pointer(pointer) {
    debug_assert(pointer, "Invalid State: Cannot create a NULL Ref object");
  }

public:

  [[nodiscard]] constexpr static Ref from_unchecked(const T* const pointer) {
    return Ref(pointer);
  }

  constexpr Ref(const T& ref): Ref(&ref) {}

  [[nodiscard]] constexpr operator const T&() const { return get_ref(); };

  [[nodiscard]] constexpr operator const T*() const { return as_ptr(); };

  [[nodiscard]] constexpr const T& operator*() const { return get_ref(); }

  [[nodiscard]] constexpr const T* operator->() const { return as_ptr(); }

  /**
   * Gets underlying pointer, this pointer is always non null
   */
  [[nodiscard]] constexpr const T* as_ptr() const { return pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] constexpr const T& get_ref() const { return *pointer; }

  friend constexpr auto operator<<(std::ostream& os, const Ref& val)
    -> std::ostream& {
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
  constexpr explicit RefMut(T* const pointer): pointer(pointer) {
    debug_assert(pointer, "Invalid State: Cannot create a NULL RefMut object");
  }

public:

  constexpr RefMut(T& ref): RefMut(&ref) {}

  [[nodiscard]] constexpr static RefMut from_unchecked(T* const pointer) {
    return RefMut(pointer);
  }

  [[nodiscard]] constexpr operator T&() const { return get_mut_ref(); };

  [[nodiscard]] constexpr operator T*() const { return as_ptr(); };

  [[nodiscard]] constexpr operator Ref<T>() const { return as_ref(); };

  [[nodiscard]] constexpr T& operator*() const { return get_mut_ref(); }

  [[nodiscard]] constexpr T* operator->() const { return as_ptr(); }

  /**
   * Gets underlying pointer, this pointer is always non null
   *
   */
  [[nodiscard]] constexpr T* as_ptr() const { return pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] constexpr T& get_mut_ref() const { return *pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] constexpr const T& get_ref() const { return *pointer; }

  /**
   * Converts RefMut into a immutable reference
   */
  [[nodiscard]] constexpr Ref<T> as_ref() const {
    return Ref<T>(get_mut_ref());
  }

  friend constexpr auto operator<<(std::ostream& os, const RefMut& val)
    -> std::ostream& {
    if constexpr (requires(const T& val) { os << val; }) {
      return os << *val;
    } else {
      return os << val.as_ptr();
    }
  }

private:

  T* pointer;
};

template<typename T>
struct std::hash<Ref<T>> {
  [[nodiscard]] auto operator()(const Ref<T>& mut) const -> usize {
    return static_cast<usize>(static_cast<uptr>(mut.as_ptr()));
  };
};

template<typename T>
struct std::hash<RefMut<T>> : std::hash<Ref<T>> {};

template<typename T>
class Option;

#include "option.hpp"

namespace crab::ref {
  /**
   * Converts a T* to a optional reference (these two types are isomorphic)
   */
  template<typename T>
  [[nodiscard]] constexpr auto from_ptr(T* const from) -> Option<T&> {
    return crab::then(from != nullptr, [from]() -> T& { return *from; });
  }

  /**
   * Converts a const T* to a optional reference (these two types are
   * isomorphic)
   */
  template<typename T>
  [[nodiscard]]
  constexpr auto from_ptr(const T* const from) -> Option<const T&> {
    return crab::then(from != nullptr, [from]() -> const T& { return *from; });
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]]
  constexpr auto cast(const Base& from) -> Option<const Derived&> {
    return from_ptr(dynamic_cast<const Derived*>(&from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]]
  constexpr auto cast(Base& from) -> Option<Derived&> {
    return from_ptr(dynamic_cast<Derived*>(&from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]]
  constexpr auto cast(const Base* from) -> Option<const Derived&> {
    return from_ptr(dynamic_cast<const Derived*>(from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]] constexpr auto cast(Base* from) -> Option<Derived&> {
    return from_ptr(dynamic_cast<Derived*>(from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]] constexpr auto cast(Ref<Base> from) -> Option<Ref<Derived>> {
    return cast<Derived, Base>(*from).template map<Ref<Derived>>();
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]] constexpr Option<RefMut<Derived>> cast(RefMut<Base> from) {
    return cast<Derived, Base>(*from).template map<RefMut<Derived>>();
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]]
  constexpr auto cast(Option<Ref<Base>> from) -> Option<Ref<Derived>> {
    return from.flat_map([](Ref<Base> base) -> Option<Ref<Derived>> {
      return cast<Derived, Base>(base);
    });
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]]
  constexpr auto cast(Option<RefMut<Base>> from) -> Option<Ref<Derived>> {
    return from.flat_map([](RefMut<Base> base) -> Option<RefMut<Derived>> {
      return cast<Derived, Base>(base);
    });
  }

  /**
   * @brief Checks if the given object is an instance of some type
   *
   * @tparam Derived What type to check
   * @param obj Object to check
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]] constexpr auto is(const Base& obj) -> bool {
    return dynamic_cast<const Derived*>(&obj) != nullptr;
  }

  /**
   * @brief Is this given parameter *exactly* this type.
   *
   * This will not perform a recursive check like dynamic_cast
   *
   * ```cpp
   *
   * class A {};
   *
   * class B: public A {};
   *
   * class C: public B {};
   *
   * i32 main() {
   *   A a;
   *   B b;
   *
   *   debug_assert(crab::ref::is_exact<A>(a),"");
   *   debug_assert(crab::ref::is_exact<B>(b),"");
   *
   *   debug_assert(crab::ref::is<A>(c),"");
   *   debug_assert(not crab::ref::is_exact<A>(c),"");
   * }
   *
   *
   * ```
   *
   * @tparam Derived
   * @param obj
   * @return
   */
  template<typename T>
  [[nodiscard]] constexpr auto is_exact(const auto& obj) -> bool {
    return typeid(obj) == typeid(T);
  }
} // namespace crab::ref

// NOLINTEND(*explicit)
