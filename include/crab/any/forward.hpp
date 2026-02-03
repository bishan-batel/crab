/// @file crab/any/forward.hpp
#pragma once

#include "crab/core.hpp"

namespace crab::any {
  template<typename... Ts>
  class AnyOf;
}

namespace crab::prelude {
  using any::AnyOf;
}

CRAB_PRELUDE_GUARD;
