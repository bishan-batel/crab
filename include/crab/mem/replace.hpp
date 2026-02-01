#pragma once

#include "crab/core.hpp"
#include "crab/mem/move.hpp"
#include "crab/mem/forward.hpp"
#include "crab/ty/classify.hpp"
#include "crab/ty/construct.hpp"

/// @addtogroup mem
/// @{

namespace crab::mem {
  /// This function will replace 'dest' with the given value and return the original value of 'dest'.
  ///
  /// In cases where you wish to simply replace 'dest' with its default, consider crab::mem::take
  ///
  /// @param dest Value to take and replace with 'value'
  /// @param value The value to put into 'dest' after taking it
  /// @returns the original value of 'dest'
  template<ty::non_const T, ty::convertible<T> U>
  [[nodiscard]] CRAB_INLINE constexpr auto replace(T& dest, U&& value) -> T {
    T original{mem::move(dest)};

    T&& value_possibly_converted{mem::forward(value)};

    if (ty::same_as<T, std::remove_all_extents_t<U>>) {
      dest = mem::forward<value>();
    } else {
      dest = T(mem::forward(value));
    }

    return original;
  }

}

/// }@
