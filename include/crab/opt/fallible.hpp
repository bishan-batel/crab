#pragma once

#include "crab/core.hpp"
#include "crab/opt/Option.hpp"
#include "crab/opt/impl/fallible.hpp"

namespace crab::opt {
  template<typename... F>
  [[nodiscard]] CRAB_INLINE constexpr auto fallible(F&&... fallible) {
    return impl::fallible{}(std::tuple<>{}, mem::forward<F>(fallible)...);
  }
}

namespace crab {
  using opt::fallible;
}
