#pragma once

#include "crab/core.hpp"
#include "unit.hpp"

namespace crab {

  /**
   * Used to discard / explicitly ignore certain outputs
   *
   * This returns unit in the case of in templated contexts you want this variadic to be assigned to something.
   */
  [[maybe_unused]] CRAB_INLINE constexpr auto discard([[maybe_unused]] auto&&...) -> unit {
    return {};
  }
}
