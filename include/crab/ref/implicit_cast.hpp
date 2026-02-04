/// @file crab/ref/implicit_cast.hpp

#pragma once

#include <type_traits>
#include "crab/core.hpp"
#include "crab/ty/identity.hpp"

namespace crab::ref {

  /// Explicit way of coercing a value to implicitly cast to a type rather than
  /// using static_cast. This symbol is also exposed as simply crab::implicit_cast
  /// @ingroup ref
  template<typename T>
  [[nodiscard]] CRAB_INLINE constexpr auto implicit_cast(
    ty::identity<T> type
  ) noexcept(std::is_nothrow_move_constructible_v<T>) -> decltype(auto) {
    return type;
  }

}

namespace crab {
  using crab::ref::implicit_cast;
}
