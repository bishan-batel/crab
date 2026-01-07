#pragma once

#include "crab/preamble.hpp"

// required for std::addressof on MSVC, no public API like __builtin_addressof
#if CRAB_MSVC_VERSION
#include <memory>
#endif

namespace crab::mem {
  /**
   * Retrieves the address / pointer to the given value, bypassing any operator& overloads
   */
  template<typename T>
  CRAB_PURE_INLINE constexpr auto address_of(T& value) -> T* {
#if CRAB_GCC_VERSION || CRAB_CLANG_VERSION
    return __builtin_addressof(value);
#else
    std::addressof(value);
#endif
  }

  template<typename T>
  CRAB_PURE_INLINE constexpr auto address_of(T&& value) -> T* = delete;
}
