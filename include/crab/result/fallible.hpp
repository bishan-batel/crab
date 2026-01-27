#pragma once

#include "crab/result/concepts.hpp"
#include "./impl/fallible.hpp"

namespace crab::result {
  template<error_type E, std::invocable... F>
  [[nodiscard]] CRAB_INLINE constexpr auto fallible(F&&... fallible) {
    return impl::fallible<E>{}(Tuple<>{}, mem::forward<F>(fallible)...);
  }
}

namespace crab {
  using result::fallible;
}
