/// @file crab/ops/Div.hpp

#pragma once

#include <utility>

/// @addtogroup ops
/// @{

namespace crab::ops {

  /// Constraint for the existence of a (binary) operator- overload between a value of a type between another
  ///
  /// @tparam T
  template<typename T, typename U = T>
  concept Div = requires(T x, U y) { x / y; };

  /// Evaluated type for the result of dividng two values (of type T, U) together
  ///
  /// @tparam T
  template<typename T, typename U = T>
  requires Div<T, U>
  using DivOutput = decltype(std::declval<T>() / std::declval<U>());

}

/// }@
