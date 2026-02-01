#pragma once

#include "crab/core.hpp"
#include "crab/ty/manipulate.hpp"

/// @addtogroup mem 
/// @{

namespace crab {
  namespace mem {

    /// Utility in templated contexts for perfect forwarding with restrictions for rvalue to lvalue conversions
    template<typename T>
    [[nodiscard]] CRAB_INLINE constexpr auto forward(ty::remove_reference<T>& value) -> T&& {
      return static_cast<T&&>(value);
    }

    /// Utility in templated contexts for perfect forwarding with restrictions for rvalue to lvalue conversions
    template<typename T>
    [[nodiscard]] CRAB_INLINE constexpr auto forward(ty::remove_reference<T>&& value) -> T&& {
      static_assert(
        not std::is_lvalue_reference_v<T>,
        "You cannot convert an rvalue to a lvalue with crab::mem::forward"
      );

      return static_cast<T&&>(value);
    }
  }

  using mem::forward;
}

/// }@
