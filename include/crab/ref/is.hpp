#pragma once

#include <concepts>
#include "crab/core.hpp"
#include "crab/mem/address_of.hpp"

namespace crab::ref {

  /**
   * @brief Checks if the given object is an instance of some type
   *
   * @tparam Derived What type to check
   * @param obj Object to check
   */
  template<class Derived, std::derived_from<Derived> Base>
  [[nodiscard]] CRAB_INLINE constexpr auto is(const Base& obj) noexcept -> bool {
    return dynamic_cast<const Derived*>(mem::address_of(obj)) != nullptr;
  }
}
