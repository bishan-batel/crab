#pragma once

#include "crab/core.hpp"

#include <functional>

namespace crab {

  /**
   * (Alias) Functor wrapper 
   */
  template<typename F = void()>
  using Func = std::function<F>;

}

namespace crab::prelude {
  using crab::Func;
}

CRAB_PRELUDE_GUARD;
