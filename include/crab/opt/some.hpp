/// @file crab/opt/some.hpp
/// @ingroup opt

#pragma once

#include <type_traits>
#include "crab/opt/Option.hpp"
#include "crab/mem/move.hpp"
#include "crab/mem/forward.hpp"
#include "crab/ty/identity.hpp"

namespace crab::opt {

  /// Creates an Option<T> from some value T (with explicit type T). This is useful when constructing an Option with a
  /// reference type, as type deduction will not try to perfect forward your reference by default for the other overload
  /// of `crab::some`.
  ///
  /// This symbol is also exposed as simply `crab::some`.
  ///
  /// # Examples
  ///
  /// ```cpp
  /// i32 a = 10;
  /// auto x = crab::some<i32&>(a);
  ///
  /// static_assert(crab::ty::same_as<x, Option<i32&>);
  ///
  /// crab_check(x.is_some());
  /// crab_check(&x.get() == &a);
  /// ```
  ///
  /// @ingroup opt
  /// @related Option
  template<typename T>
  [[nodiscard]] CRAB_INLINE constexpr auto some(ty::identity<T>&& from) -> Option<T> {
    return Option<T>{mem::forward<T>(from)};
  }

  /// Creates an Option<T> from some value T.
  ///
  /// This symbol is also exposed as simply `crab::some`. Note that if you do not specify the template argument, this
  /// will always assume the option you are trying to construct is one of a *non reference type*.
  ///
  /// # Examples
  ///
  /// ```cpp
  /// i32 a = 10;
  /// auto x = crab::some(a);
  ///
  /// static_assert(crab::ty::same_as<x, Option<i32>);
  ///
  /// crab_check(x.is_some());
  /// crab_check(x.get() == 10);
  /// ```
  ///
  /// @ingroup opt
  /// @related Option
  [[nodiscard]] CRAB_INLINE constexpr auto some(auto from) {
    return Option<std::remove_cvref_t<decltype(from)>>{mem::move(from)};
  }
}

namespace crab {
  using opt::some;
}
