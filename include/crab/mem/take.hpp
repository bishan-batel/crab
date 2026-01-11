#pragma once

#include "crab/type_traits.hpp"
#include "./move.hpp"

namespace crab::mem {

  namespace impl {
    template<typename T>
    concept takeable = ty::non_const<T> and ty::movable<T> and ty::default_constructible<T>;

    template<typename T>
    concept take_noexcept = std::is_nothrow_move_constructible_v<T> and std::is_nothrow_move_assignable_v<T>
                        and std::is_nothrow_default_constructible_v<T>;
  }

  /**
   * Moves the given value and returns, reassigning its default value.
   *
   * Analogous to `mem::replace<T>(value, T())`
   */
  template<ty::movable T>
  requires std::is_default_constructible_v<T>
  CRAB_NODISCARD_INLINE_CONSTEXPR auto take(T& value) noexcept(impl::take_noexcept<T>) -> T {
    T temp{mem::move(value)};
    value = T();
    return temp;
  }

}
