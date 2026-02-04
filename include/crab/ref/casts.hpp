/// @file crab/ref/casts.hpp
/// @ingroup ref

#pragma once

#include <type_traits>

#include "crab/opt/Option.hpp"

#include "crab/ref/from_ptr.hpp"
#include "crab/ref/ref.hpp"

namespace crab::ref {
  /// @addtogroup ref
  /// @{

  // Attempts to cast input of type Base into a Derived instance
  template<class Derived, typename Base>
  [[nodiscard]] CRAB_INLINE constexpr auto cast(const Base* from) -> opt::Option<const Derived&> {
    static_assert(std::derived_from<Derived, Base>);
    return from_ptr(dynamic_cast<const Derived*>(from));
  }

  /// @copydoc crab::ref::cast
  template<class Derived, typename Base>
  [[nodiscard]] CRAB_INLINE constexpr auto cast(Base* from) -> opt::Option<Derived&> {
    static_assert(std::derived_from<Derived, Base>);
    return from_ptr(dynamic_cast<Derived*>(from));
  }

  /// @copydoc crab::ref::cast
  template<class Derived, class Base>
  [[nodiscard]] CRAB_INLINE constexpr auto cast(const Base& from) -> opt::Option<const Derived&> {
    static_assert(std::derived_from<Derived, Base>);
    return cast<Derived, Base>(std::addressof(from));
  }

  /// @copydoc crab::ref::cast
  template<class Derived, class Base>
  [[nodiscard]] CRAB_INLINE constexpr auto cast(Base& from) -> opt::Option<Derived&> {
    return cast<Derived, Base>(std::addressof(from));
  }

  /// @copydoc crab::ref::cast
  template<class Derived, typename Base>
  [[nodiscard]] CRAB_INLINE constexpr auto cast(Ref<Base> from) -> opt::Option<Ref<Derived>> {
    return cast<Derived, Base>(from.get_ref()).template map<Ref<Derived>>();
  }

  /// @copydoc crab::ref::cast
  template<class Derived, typename Base>
  [[nodiscard]] CRAB_INLINE constexpr auto cast(RefMut<Base> from) -> opt::Option<RefMut<Derived>> {
    return cast<Derived, Base>(from.get_ref()).template map<RefMut<Derived>>();
  }

  /// }@
}
