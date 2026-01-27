#pragma once

#include "crab/core.hpp"

namespace crab {

  /**
   * @brief Denotes unreachable paths
   * This should be used for optimisation purposes only.
   */
  [[noreturn]] CRAB_INLINE auto unreachable() -> void {

#if CRAB_HAS_UNREACHABLE
    std::unreachable();
#elif CRAB_MSVC_VERSION && !CRAB_CLANG_VERSION
    CRAB_ASSUME(false);
#elif CRAB_CLANG_VERSION || CRAB_GCC_VERSION
    __builtin_unreachable();
#else
    CRAB_ASSUME(false);
    while (true);
#endif
  }
}
