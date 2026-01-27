#pragma once

#include "crab/core.hpp"
#include <source_location>

namespace crab {

  /**
   * Alias for std::source_location
   */
  using SourceLocation = std::source_location;

}

namespace crab::prelude {
  using crab::SourceLocation;
}

CRAB_PRELUDE_GUARD;
