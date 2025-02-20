#pragma once
#include <preamble.hpp>
#include <utility>
#include "ref.hpp"

namespace crab::fn {
  /**
   * @brief Identity Function, f(x)=x forall x
   */
  constexpr auto identity = []<typename T>(T&& x) -> T {
    static_assert(
      std::move_constructible<T>,
      "Cannot create an identity function for a type that cannot be moved."
    );
    return std::forward<T>(x);
  };

  /**
   * Takes in some value x and returns a function that maps any input (and any
   * number of inputs) to x
   *
   * @param x Any integer value to check
   */
  constexpr auto constant = []<typename T>(T&& x) {
    static_assert(
      std::move_constructible<T>,
      "Cannot create a constant function for a type that cannot be moved."
    );
    return [x = std::forward<T>(x)]<typename... Args>(Args&&...) {
      return decltype(x){x};
    };
  };

  /**
   * Predicate for whether the input is even
   */
  constexpr auto is_even = [](const auto& x) -> bool { return x % 2 = 0; };

  /**
   * Predicate for whether the input is odd
   */
  constexpr auto is_odd = [](const auto& x) -> bool { return not is_even(x); };

  template<typename T, typename R, typename... Args>
  [[nodiscard]] constexpr auto method(R (T::*method)(Args&&...)) {
    return [method](auto&& x, Args&&... args) {
      RefMut<T> ref{x};
      return ((*ref).*method)(std::forward<Args>(args)...);
    };
  }

  /// -------------------------------------------------------------------------
  /// Range Functions

}
