/// @file crab/ops/Rem.hpp
#pragma once

#include <utility>

/// @addtogroup ops
/// @{

namespace crab::ops {

  /// Constraint for the existence of a remainder operator% overload between a value of a type between another
  template<typename T, typename U = T>
  concept Rem = requires(T x, U y) { x % y; };

  /// Evaluated type for the result of remaining (?) two values (of type T, U) together
  template<typename T, typename U = T>
  requires Rem<T, U>
  using RemOutput = decltype(std::declval<T>() % std::declval<U>());

}

/// }@
