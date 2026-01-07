#pragma once

#include <type_traits>
#include "crab/type_traits.hpp"
#include "./move.hpp"
#include "./address_of.hpp"

namespace crab::mem {
  template<typename T>
  concept trivially_reloctable = std::is_trivially_copyable_v<T>;

  namespace impl {
    template<ty::non_const T>
    CRAB_CONSTEXPR auto swap_non_trivial(T& lhs, T& rhs) -> void {
      T temp{mem::move(lhs)};
      lhs = mem::move(rhs);
      rhs = mem::move(temp);
    }

    template<ty::non_const T>
    CRAB_CONSTEXPR auto swap_trivial_relocatable(T& lhs, T& rhs) -> void {
      // TODO:
      swap_non_trivial(lhs, rhs);
    }
  }

  // clang-format on

  template<ty::non_const T>
  CRAB_CONSTEXPR auto swap_nonoverlapping(T& __restrict lhs, T& __restrict rhs) -> void {
    if constexpr (not trivially_reloctable<T>) {
      impl::swap_non_trivial(lhs, rhs);
      return;
    }

    if (std::is_constant_evaluated()) {
      impl::swap_non_trivial(lhs, rhs);
    } else {
      impl::swap_trivial_relocatable(lhs, rhs);
    }
  }

  template<ty::non_const T>
  CRAB_CONSTEXPR auto swap(T& lhs, T& rhs) noexcept(
    std::is_nothrow_move_assignable_v<T> and std::is_nothrow_move_constructible_v<T>
  ) -> void {
    if constexpr (not trivially_reloctable<T>) {
      impl::swap_non_trivial(lhs, rhs);
      return;
    }

    if (std::is_constant_evaluated()) {
      impl::swap_non_trivial(lhs, rhs);
      return;
    }

    if (address_of(lhs) != address_of(rhs)) [[likely]] {
      impl::swap_trivial_relocatable(lhs, rhs);
      return;
    }
  }
}
