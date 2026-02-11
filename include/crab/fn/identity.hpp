/// @file crab/fn/identity.hpp

#pragma once

#include <concepts>

#include "crab/mem/forward.hpp"
#include "crab/mem/move.hpp"
#include "crab/ty/construct.hpp"

namespace crab::fn {
  /// Identity Function, f(x)=x forall x
  constexpr auto identity{
    []<typename T>(T&& x) { return mem::forward<T>(x); },
  };

  /// Takes in some value x and returns a function that maps any input (and any
  /// number of inputs) to x
  ///
  /// @param x Any integer value to check
  constexpr auto constant{
    []<ty::copy_constructible T>(T x) {
      return [x = mem::move(x)]<typename... Args>(Args&&...) -> T { return T(x); };
    },
  };

}
