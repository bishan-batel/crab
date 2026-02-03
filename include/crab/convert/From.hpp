/// @file crab/convert/From.hpp

#pragma once

#include <concepts>

#include "crab/mem/forward.hpp"
#include "crab/ty/compare.hpp"
#include "crab/ty/construct.hpp"

namespace crab::conv {

  namespace impl {
    template<typename T, typename F>
    concept HasFromMethod = requires {
      { T::from(std::declval<F>()) } -> ty::same_as<T>;
    };
  }

  /// Constraint that a value of type T must be able to be constructible with a value of F, or have a factory method
  /// 'from' that takes F.
  ///
  /// To properly apply the use of this concept, see crab::conv::from
  /// st
  /// ```
  ///
  /// @tparam T Output type
  /// @tparam F Input type
  template<typename T, typename F>
  concept From = ty::convertible<F, T> or impl::HasFromMethod<T, F>;

  /// Performs conversion between a value of F -> T
  ///
  /// @tparam T Produced type
  /// @tparam F Input type
  /// @param value Value to be converted
  /// @returns Converted type
  template<typename T, typename F>
  requires From<T, F>
  [[nodiscard]] constexpr auto from(F&& value) -> T {
    if constexpr (impl::HasFromMethod<T, F>) {
      return T::from(mem::forward<F>(value));
    } else {
      return static_cast<T>(mem::forward<F>(value));
    }
  }

}

namespace crab {
  using crab::conv::from;
}
