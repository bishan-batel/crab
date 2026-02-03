/// @file crab/ops/Add.hpp

#pragma once

#include <utility>

/// @addtogroup ops
/// @{

namespace crab::ops {

  /// Constraint for the existence of a operator+ overload between a value of a type between another
  ///
  /// @tparam T
  template<typename T, typename U = T>
  concept Add = requires(T x, U y) { x + y; };

  /**
   * Evaluated type for the result of adding two values (of type T, U) together
   *
   * @tparam T
   */
  template<typename T, typename U = T>
  requires Add<T, U>
  using AddOutput = decltype(std::declval<T>() + std::declval<U>());
}

/// }@
