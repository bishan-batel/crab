/// @file crab/opt/boolean_constructs.hpp
/// @ingroup opt

#pragma once

#include "crab/core.hpp"
#include "crab/mem/forward.hpp"
#include "crab/mem/move.hpp"
#include "crab/opt/Option.hpp"

namespace crab::opt {
  /// Maps a boolean to an option if the given condition is true
  ///
  /// # Examples
  /// ```cpp
  /// Option<i32> value = crab::then(true, []() { return 10; });
  /// crab_check(value.is_some());
  /// crab_check(crab::move(value).unwrap() == 10);
  ///
  /// value = crab::then(false, []() { return 10; });
  /// crab_check(value.is_none());
  /// ```
  /// @ingroup opt
  template<ty::provider F>
  [[nodiscard]] CRAB_INLINE constexpr auto then(const bool cond, F&& func) {
    using Return = Option<ty::functor_result<F>>;

    if (not cond) {
      return Return{};
    }

    return Return{std::invoke(func)};
  }

  /// Maps a boolean to an option if the given condition is false.
  ///
  /// @param cond Condition determining whether the func will be invoked or not
  ///
  /// # Examples
  /// ```cpp
  /// Option<i32> value = crab::unless(false, []() { return 10; });
  /// crab_check(value.is_some());
  /// crab_check(crab::move(value).unwrap() == 10);
  ///
  /// value = crab::unless(true, []() { return 10; });
  /// crab_check(value.is_none());
  /// ```
  /// @ingroup opt
  template<ty::provider F>
  [[nodiscard]] CRAB_INLINE constexpr auto unless(const bool cond, F&& func) {
    using Return = Option<ty::functor_result<F>>;

    if (cond) {
      return Return{};
    }

    return Return{std::invoke(func)};
  }

  /// Consumes given option and returns the contained value, this will panic if the option is empty.
  ///
  /// This is equivalent to just moving the option and calling unwrap.
  ///
  /// @param from Option to consume
  ///
  /// # Panics
  /// If the given option is none then this will panic.
  ///
  /// @ingroup opt
  template<typename T>
  [[nodiscard]] CRAB_INLINE constexpr auto unwrap(Option<T>&& from) -> T {
    return mem::move<T>(from).unwrap();
  }
}

namespace crab {
  using opt::then;
  using opt::unless;
  using opt::unwrap;
}
