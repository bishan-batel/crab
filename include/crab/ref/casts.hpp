#pragma once

#include <type_traits>

#include "crab/opt/Option.hpp"

#include "crab/ref/from_ptr.hpp"
#include "crab/ref/ref.hpp"

namespace crab::ref {

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, class Base>
  [[nodiscard]] CRAB_INLINE constexpr auto cast(const Base& from) -> opt::Option<const Derived&> {
    static_assert(std::derived_from<Derived, Base>);
    return from_ptr(dynamic_cast<const Derived*>(std::addressof(from)));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, class Base>
  [[nodiscard]] CRAB_INLINE constexpr auto cast(Base& from) -> opt::Option<Derived&> {
    static_assert(std::derived_from<Derived, Base>);
    return from_ptr(dynamic_cast<Derived*>(std::addressof(from)));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]] CRAB_INLINE constexpr auto cast(const Base* from) -> opt::Option<const Derived&> {
    static_assert(std::derived_from<Derived, Base>);
    return from_ptr(dynamic_cast<const Derived*>(from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]] CRAB_INLINE constexpr auto cast(Base* from) -> opt::Option<Derived&> {
    static_assert(std::derived_from<Derived, Base>);
    return from_ptr(dynamic_cast<Derived*>(from));
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]] CRAB_INLINE constexpr auto cast(Ref<Base> from) -> opt::Option<Ref<Derived>> {
    static_assert(std::derived_from<Derived, Base>);
    return cast<Derived, Base>(from.get_ref()).template map<Ref<Derived>>();
  }

  /**
   * @brief Attempts to cast input of type Base into a Derived instances
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]] CRAB_INLINE constexpr opt::Option<RefMut<Derived>> cast(RefMut<Base> from) {
    return cast<Derived, Base>(from.get_ref()).template map<RefMut<Derived>>();
  }

}

// namespace crab::ref
