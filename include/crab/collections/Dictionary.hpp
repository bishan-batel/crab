#pragma once

#include <unordered_map>
#include "crab/core.hpp"

namespace crab {
  /**
   * @brief Unordered key-value collection
   */
  template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Predicate = std::equal_to<Key>>
  using Dictionary = std::unordered_map<Key, Value, Hash, Predicate>;
}

namespace crab::prelude {
  using crab::Dictionary;
}

CRAB_PRELUDE_GUARD;
