/// @file crab/mem/swap.hpp

#pragma once

#include <type_traits>

#include "crab/core.hpp"
#include "crab/mem/impl/swap.hpp"
#include "crab/ty/classify.hpp"

/// @addtogroup mem
/// @{

namespace crab::mem {

  /// Whether or not a type can be trivially relocatable
  /// TODO: find a way reliably detect relocatability
  template<typename T>
  concept trivially_reloctable = std::is_trivially_destructible_v<T>    //
                             and std::is_trivially_move_assignable_v<T> //
                             and std::is_trivially_move_constructible_v<T>;

  // clang-format on

  /// Swaps the given values template<ty::non_const T>. The difference between this and std::swap is this function will
  /// perform bitwise relocation if applicable for the given type, rather than call move assignment or constructors.
  ///
  /// This function is comutative.
  ///
  /// @param lhs value to swap
  /// @param rhs value to swap
  template<ty::non_const T>
  constexpr auto swap(T& lhs, T& rhs) noexcept(
    std::is_nothrow_move_assignable_v<T> and std::is_nothrow_move_constructible_v<T>
  ) -> void {

    if constexpr (not trivially_reloctable<T>) {
      impl::swap_non_trivial(lhs, rhs);
      return;
    }

    /// unable to memcpy in constant evaluated contexts, do this to perserve constexpr compatability
    crab_if_consteval {
      impl::swap_non_trivial(lhs, rhs);
      return;
    }

    if (address_of(lhs) != address_of(rhs)) [[likely]] {
      impl::swap_trivial_relocatable(unsafe, lhs, rhs);
      return;
    }
  }
}

/// }@
