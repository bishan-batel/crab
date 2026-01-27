#pragma once

#include "crab/core.hpp"
#include "crab/opt/Option.hpp"

namespace crab::ref {

  /**
   * Converts a pointer into an optional reference
   */
  template<typename T>
  [[nodiscard]] CRAB_INLINE constexpr auto from_ptr(T* from) -> opt::Option<T&> {
    if (from == nullptr) {
      return {};
    }

    return opt::Option<T&>{*from};
  }

  /**
   * Converts a const T* to a optional reference
   */
  template<typename T>
  [[nodiscard]] CRAB_INLINE constexpr auto from_ptr(const T* from) -> opt::Option<const T&> {
    if (from != nullptr) {
      return opt::Option<const T&>{*from};
    }

    return {};
  }
}
