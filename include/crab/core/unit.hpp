#pragma once

#include "crab/core.hpp"

#include <ostream>

namespace crab {

  /**
   * @brief Monostate type, all instances of 'unit' are indistinguishable,
   * note that this type will never be '0' sized, unless being used as a field
   * with [[no_unique_address]]
   */
  struct unit final {
    /**
     * Instance of unit, equivalent to constructing a new one, more of a stylistic
     * choice whether you want to type unit{} or unit::val, I personally use
     * unit::val to imply that there is a single value of unit
     */
    static const unit val;

    /**
     * Unit has not state, all instances of until are equal
     * (this will always return true)
     */
    CRAB_PURE CRAB_INLINE constexpr auto operator==(const unit&) const -> bool {
      return true;
    }
  };

  inline const unit unit::val{};

  namespace prelude {
    using crab::unit;
  }
}

inline auto operator<<(std::ostream& os, ::crab::unit) -> decltype(auto) {
  return os << "unit";
}

CRAB_PRELUDE_GUARD;
