#pragma once

#include <vector>
#include "crab/core.hpp"

namespace crab {
  /**
   * @brief (Alias) Heap allocated, dynamically sized list
   */
  template<typename T>
  using Vec = std::vector<T>;
}

namespace crab::prelude {
  using crab::Vec;
}

CRAB_PRELUDE_GUARD;
