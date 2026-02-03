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
  /// @ingroup opt
  template<typename T>
  [[nodiscard]] CRAB_INLINE constexpr auto some(ty::identity<T>&& from) {
    return Option<T>{mem::forward<T>(from)};
  }

  /// Creates an Option<T> from some value T.
  ///
  /// This symbol is also exposed as simply `crab::some`.
  ///
  /// @ingroup opt
  [[nodiscard]] CRAB_INLINE constexpr auto some(auto from) {
    return Option<std::remove_cvref_t<decltype(from)>>{mem::move(from)};
  }
}

namespace crab {
  using opt::some;
}
