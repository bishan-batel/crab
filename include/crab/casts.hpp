#pragma once

#include <crab/ref.hpp>
#include <type_traits>
#include "crab/opt/Option.hpp"

namespace crab {

  namespace ref {
    /**
     * Converts a pointer into an optional reference
     */
    template<typename T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto from_ptr(T* from) -> opt::Option<T&> {
      if (from == nullptr) {
        return {};
      }

      return opt::Option<T&>{*from};
    }

    /**
     * Converts a const T* to a optional reference
     */
    template<typename T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto from_ptr(const T* from) -> opt::Option<const T&> {
      if (from != nullptr) {
        return opt::Option<const T&>{*from};
      }

      return {};
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, class Base>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto cast(const Base& from) -> opt::Option<const Derived&> {
      static_assert(std::derived_from<Derived, Base>);
      return from_ptr(dynamic_cast<const Derived*>(std::addressof(from)));
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, class Base>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto cast(Base& from) -> opt::Option<Derived&> {
      static_assert(std::derived_from<Derived, Base>);
      return from_ptr(dynamic_cast<Derived*>(std::addressof(from)));
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, std::derived_from<Derived> Base>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto cast(const Base* from) -> opt::Option<const Derived&> {
      static_assert(std::derived_from<Derived, Base>);
      return from_ptr(dynamic_cast<const Derived*>(from));
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, std::derived_from<Derived> Base>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto cast(Base* from) -> opt::Option<Derived&> {
      static_assert(std::derived_from<Derived, Base>);
      return from_ptr(dynamic_cast<Derived*>(from));
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, std::derived_from<Derived> Base>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto cast(Ref<Base> from) -> opt::Option<Ref<Derived>> {
      static_assert(std::derived_from<Derived, Base>);
      return cast<Derived, Base>(from.get_ref()).template map<Ref<Derived>>();
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, std::derived_from<Derived> Base>
    CRAB_NODISCARD_INLINE_CONSTEXPR opt::Option<RefMut<Derived>> cast(RefMut<Base> from) {
      return cast<Derived, Base>(from.get_ref()).template map<RefMut<Derived>>();
    }

    /**
     * @brief Checks if the given object is an instance of some type
     *
     * @tparam Derived What type to check
     * @param obj Object to check
     */
    template<class Derived, std::derived_from<Derived> Base>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto is(const Base& obj) noexcept -> bool {
      const auto* casted{dynamic_cast<const Derived*>(std::addressof(obj))};
      return casted != nullptr;
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
    CRAB_NODISCARD_INLINE_CONSTEXPR auto is_exact(const auto& obj) noexcept -> bool {
      return typeid(obj) == typeid(T);
    }
  }

  /**
   * Explicit way of coercing a value to implicitly cast to a type rather than
   * using static_cast
   */
  template<typename T>
  CRAB_NODISCARD_INLINE_CONSTEXPR auto implicit_cast(ty::identity<T> type) //
    noexcept(std::is_nothrow_move_constructible_v<T>) -> decltype(auto) {
    return type;
  }

} // namespace crab::ref
