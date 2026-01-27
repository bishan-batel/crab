#pragma once

#include <unordered_set>
#include "crab/core.hpp"

namespace crab {
  /**
   * @brief (Alias) Unordered set of elements
   */
  template<typename T, typename Hash = std::hash<T>, typename Predicate = std::equal_to<T>>
  using Set = std::unordered_set<T, Hash, Predicate>;
}

namespace crab::prelude {
  using crab::Set;
}

CRAB_PRELUDE_GUARD;
