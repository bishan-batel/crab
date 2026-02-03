/// @file crab/ty/construct.hpp
/// @ingroup ty
/// Type constraints related to construction of types & move semantics.
#pragma once

#include <concepts>

namespace crab::ty {
  /// @addtogroup ty
  /// @{

  /// Requirement for the type T to be copy constructible and assignable
  template<typename T>
  concept copyable = std::copyable<T>;

  /// Requirement for the type T to be copy constructible
  template<typename T>
  concept copy_constructible = std::copy_constructible<T>;

  /// Requirement for the type T to be copy assignable
  template<typename T>
  concept copy_assignable = std::is_copy_assignable_v<T>;

  /// Requirement that From can be converted into To with static_cast by some means
  template<typename From, typename To>
  concept convertible = std::convertible_to<From, To>;

  /// Requirement that From can be noexcept converted into To with static_cast by some means
  template<typename From, typename To>
  concept convertible_nothrow = std::is_nothrow_convertible_v<From, To>;

  /// Requirement for T to be a movable type (constructible and assignable)
  template<typename T>
  concept movable = std::movable<T>;

  /// Requirement for T to be default constructible
  template<typename T>
  concept default_constructible = std::is_default_constructible_v<T>;

  /// }@
}
