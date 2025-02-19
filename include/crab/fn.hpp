#pragma once
#include <preamble.hpp>
#include <utility>
#include "ref.hpp"

namespace crab::fn {
  /**
   * @brief Identity Function, f(x)=x forall x
   */
  [[nodiscard]] constexpr auto identity(auto&& x) -> decltype(x) {
    static_assert(
      std::move_constructible<decltype(x)>,
      "Cannot create an identity function for a type that cannot be moved."
    );
    return std::forward<std::remove_reference_t<decltype(x)>>(x);
  };

  /**
   * Takes in some value x and returns a function that maps any input (and any
   * number of inputs) to x
   *
   * @param x Any integer value to check
   */
  [[nodiscard]] constexpr auto constant(auto&& x) {
    static_assert(
      std::move_constructible<decltype(x)>,
      "Cannot create a constant function for a type that cannot be moved."
    );
    return [x = std::forward<decltype(x)>(x)]<typename... Args>(Args&&...) {
      return decltype(x){x};
    };
  };

  /**
   * Predicate for whether the input is even
   */
  [[nodiscard]] constexpr auto is_even(const auto& x) -> bool {
    return x % 2 = 0;
  };

  /**
   * Predicate for whether the input is odd
   */
  [[nodiscard]] constexpr auto is_odd(const auto& x) -> bool {
    return not is_even(x);
  };

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
