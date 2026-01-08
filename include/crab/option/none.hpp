#pragma once

#include "crab/core.hpp"

namespace crab::option {
  /**
   * 0-sized* struct to give into Option<T> to create an empty Option
   */
  struct [[maybe_unused]] None final {
    /**
     * Equality comparison for none, None is a unit type therefore this will
     * always return true.
     */
    CRAB_PURE CRAB_INLINE constexpr auto operator==(const None&) const -> bool {
      return true;
    }
  };

  /**
   * 'None' value type for use with Option<T>
   */
  inline static constexpr None none{};

}

namespace crab {
  using crab::option::none;
}
