#pragma once

#include "crab/type_traits.hpp"

namespace crab::mem {
  /**
   * Equivalent to std::move with the added constraint of not being able to move from const, which is almost always a
   * bug
   */
  template<typename T>
  CRAB_PURE_INLINE_CONSTEXPR auto move(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) -> decltype(auto) {
    static_assert(ty::non_const<ty::remove_reference<T>>, "Cannot move from a const value");
    return static_cast<ty::remove_reference<T>&&>(value);
  }
}

namespace crab {
  using crab::mem::move;
}
