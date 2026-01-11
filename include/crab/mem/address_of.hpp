#pragma once

#include "crab/core.hpp"

#include <memory>

namespace crab::mem {
  /**
   * Retrieves the address / pointer to the given value, bypassing any operator& overloads
   */
  template<typename T>
  CRAB_PURE_INLINE constexpr auto address_of(T& value) noexcept -> T* {
    return std::addressof(value);
  }
}
