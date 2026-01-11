#pragma once

#include "crab/core.hpp"
#include "crab/type_traits.hpp"

namespace crab {
  namespace mem {

    template<typename T>
    [[nodiscard]] CRAB_INLINE constexpr auto forward(ty::remove_reference<T>& value) -> T&& {
      return static_cast<T&&>(value);
    }

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
