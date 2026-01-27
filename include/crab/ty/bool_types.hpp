#pragma once

#include <type_traits>

namespace crab::ty {
  using true_type = std::bool_constant<true>;

  using false_type = std::bool_constant<false>;

  /**
   * @brief Conditional metafunction, if the condition is true it will select
   * IfTrue, else it will select IfFalse. You can think of this as a ternary
   * operator for Types instead of values.
   *
   * @tparam IfTrue
   * @tparam IfFalse
   */
  template<bool Condition, typename IfTrue, typename IfFalse>
  using conditional = std::conditional_t<Condition, IfTrue, IfFalse>;

}
