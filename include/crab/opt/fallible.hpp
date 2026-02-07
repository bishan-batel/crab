#pragma once

#include "crab/core.hpp"
#include "crab/opt/Option.hpp"
#include "crab/opt/concepts.hpp"
#include "crab/opt/impl/fallible.hpp"
#include "crab/ty/functor.hpp"

namespace crab::opt {
  /// This function is a utility for invoking a list of functors that each produce an option in a way that mimics 'short
  /// circuiting'. The functors passed will be invoked in order that they were passed in, however if during this chain
  /// one returns 'none' (an empty Option<std::tuple<...>), this entire function will return none. If all functors
  /// return an option with a value - then all produced values are added to a std::tuple and is returned as a
  /// Option<std::tuple<...>>.
  ///
  /// This is also exposed as crab::fallible, however if there lies ambiguity whether you are invoking
  /// crab::result::fallible vs crab::opt::fallible, you can work around this by using the full path.
  ///
  /// # Examples
  ///
  /// An example of fallible returning a valid option:
  /// ```cpp
  /// Option<Tuple<i32, f32>> value = crab::fallible(
  ///   []() { return Option<i32>(10); },
  ///   []() { return Option<f32>(20.0f); }
  /// );
  ///
  /// crab_check(value.is_some());
  ///
  /// auto [num_int, num_float] = crab::move(value).unwrap();
  /// crab_check(num_int == 10);
  /// crab_check(num_float == 20.0f);
  /// ```
  ///
  /// An example of short circuiting:
  /// ```cpp
  /// Option<Tuple<String, f32>> value = crab::fallible(
  ///   []() { return Option<String>("hello"); },
  ///   []() { return Option<f32>{}; }
  /// );
  ///
  /// crab_check(value.is_none());
  /// ```
  ///
  /// Note that not all functors passed have to return an option, if one returns a value not wrapped in an option - it
  /// is simply works the same as a functor that always return a value-filled option.
  ///
  /// ```cpp
  /// Option<Tuple<i32, f32, String>> value = crab::fallible(
  ///   []() { return Option<i32>(10); },
  ///   []() { return 20.0f; },
  ///   []() { return String{"hello"}; },
  /// );
  ///
  /// crab_check(value.is_some());
  ///
  /// auto [num_int, num_float, str] = crab::move(value).unwrap();
  /// crab_check(num_int == 10);
  /// crab_check(num_float == 20.0f);
  /// crab_check(str == "hello");
  /// ```
  ///
  /// @relates Option
  /// @ingroup opt
  template<typename... F>
  [[nodiscard]] CRAB_INLINE constexpr auto fallible(F&&... functors) {
    return impl::fallible{}(std::tuple{}, mem::forward<F>(functors)...);
  }
}

namespace crab {
  using opt::fallible;
}
