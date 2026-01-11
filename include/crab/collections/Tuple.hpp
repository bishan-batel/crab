#pragma once

#include <tuple>
#include "crab/core.hpp"

namespace crab {
  /**
   * @brief std::tuple<T...> alias.
   */
  template<typename... Types>
  using Tuple = std::tuple<Types...>;

}

namespace crab::prelude {
  using crab::Tuple;
}

CRAB_PRELUDE_GUARD;
