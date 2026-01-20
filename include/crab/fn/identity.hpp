#pragma once

#include <concepts>

#include "crab/mem/forward.hpp"

namespace crab::fn {
  /**
   * @brief Identity Function, f(x)=x forall x
   */
  constexpr auto identity{
    []<typename T>(T&& x) {
      static_assert(std::move_constructible<T>, "Cannot create an identity function for a type that cannot be moved.");
      return mem::forward<T>(x);
    },
  };

  /**
   * Takes in some value x and returns a function that maps any input (and any
   * number of inputs) to x
   *
   * @param x Any integer value to check
   */
  constexpr auto constant{
    []<ty::copy_constructible T>(T x) {
      return [x = mem::forward<T>(x)]<typename... Args>(Args&&...) -> T { return x; };
    },
  };

}
