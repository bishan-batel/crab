/// @file crab/ref/from_ptr.hpp

#pragma once

#include "crab/core.hpp"
#include "crab/opt/Option.hpp"
#include "crab/ty/classify.hpp"

namespace crab::ref {

  /// @ingroup ref
  /// Converts a pointer into an optional reference. If the pointer is null this returns 'none', else this returns an
  /// option with a reference to the value pointed at.
  ///
  /// # Examples
  ///
  /// ```cpp
  /// i32* ptr = nullptr;
  /// Option<i32&> x = crab::ref::from_ptr(ptr);
  ///
  /// crab_check(x.is_none());
  ///
  /// i32 a = 10;
  /// ptr = &a;
  ///
  /// x = crab::ref::from_ptr(ptr);
  ///
  /// crab_check(x.is_some() and crab::move(x).unwrap() == 10);
  /// ```
  template<typename T>
  [[nodiscard]] CRAB_INLINE constexpr auto from_ptr(T* from) -> opt::Option<T&> {

    if (from == nullptr) {
      return {};
    }

    return opt::Option<T&>{*from};
  }
}
