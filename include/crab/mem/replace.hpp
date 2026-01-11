#pragma once

#include "crab/core.hpp"
#include "crab/type_traits.hpp"
#include "./move.hpp"

namespace crab::mem {
  template<ty::non_const T, ty::convertible<T> U>
  CRAB_NODISCARD_INLINE_CONSTEXPR auto replace(T& dest, U&& value) -> T {
    T original{mem::move(dest)};

    T&& value_possibly_converted{value};

    dest = mem::move(value_possibly_converted);

    return original;
  }

}
