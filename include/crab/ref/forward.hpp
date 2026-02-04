/// @file crab/ref/forward.hpp
/// @ingroup ref
/// Forward declarations for select items in the crab::ref namespace

#pragma once

#include "crab/opt/forward.hpp"

namespace crab::ref {
  template<typename T>
  class Ref;

  template<typename T>
  class RefMut;

  template<class Derived, typename Base>
  constexpr auto cast(const Base* from) -> opt::Option<const Derived&>;

  template<class Derived, typename Base>
  constexpr auto cast(Base* from) -> opt::Option<Derived&>;

  template<class Derived, class Base>
  constexpr auto cast(const Base& from) -> opt::Option<const Derived&>;

  template<class Derived, class Base>
  constexpr auto cast(Base& from) -> opt::Option<Derived&>;

  template<class Derived, class Base>
  constexpr auto cast(Ref<Base> from) -> opt::Option<Ref<Derived>>;

  template<class Derived, class Base>
  constexpr auto cast(RefMut<Base> from) -> opt::Option<RefMut<Derived>>;
}
