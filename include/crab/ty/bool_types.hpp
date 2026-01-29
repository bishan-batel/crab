/// @file bool_types.hpp
/// @brief Boolean related metatemplate types.
/// @ingroup ty

#pragma once

#include <type_traits>

/// @addtogroup ty
/// @{

namespace crab::ty {
  /// A type with a constant 'value' set to true.
  /// This is the same as std::true_type, std::bool_constant<true>, or std::integral_constant<bool,true>
  using true_type = std::bool_constant<true>;

  /// A type with a constant 'value' set to false.
  /// This is the same as std::false_type, std::bool_constant<false>, or std::integral_constant<bool,false>
  using false_type = std::bool_constant<false>;

  /// Conditional metafunction. If the condition is true it will select
  /// IfTrue, else it will select IfFalse. You can think of this as a ternary
  /// operator for Types instead of values.
  ///
  /// @tparam Condition Condition to check for (true/false compile-time constant)
  /// @tparam IfTrue The type 'returned' if Condition is true
  /// @tparam IfFalse The type 'returned' if Condition is false
  ///
  /// # Examples
  /// ```cpp
  /// static_assert(typeid(crab::ty::conditional<true, float, bool>) == typeid(float));
  /// static_assert(typeid(crab::ty::conditional<false, float, bool>) == typeid(bool));
  /// ```
  template<bool Condition, typename IfTrue, typename IfFalse>
  using conditional = std::conditional_t<Condition, IfTrue, IfFalse>;

}

/// }@
