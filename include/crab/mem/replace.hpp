#pragma once

#include "crab/core.hpp"
#include "./move.hpp"
#include "crab/ty/classify.hpp"
#include "crab/ty/construct.hpp"

namespace crab::mem {
  template<ty::non_const T, ty::convertible<T> U>
  [[nodiscard]] CRAB_INLINE constexpr auto replace(T& dest, U&& value) -> T {
    T original{mem::move(dest)};

    T&& value_possibly_converted{value};

    dest = mem::move(value_possibly_converted);

    return original;
  }

}
