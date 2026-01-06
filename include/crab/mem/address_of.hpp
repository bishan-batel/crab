#pragma once

#include "crab/preamble.hpp"

namespace crab::mem {
  /**
   * Retrieves the address / pointer to the given value, bypassing any operator& overloads
   */
  template<typename T>
  CRAB_PURE_INLINE_CONSTEXPR auto address_of(T& value) -> T* {
#if CRAB_GCC_VERSION || CRAB_CLANG_VERSION
    return __builtin_addressof(value);
#else
    std::addressof(value);
#endif
  }

  template<typename T>
  CRAB_PURE_INLINE_CONSTEXPR auto address_of(T&& value) -> T* = delete;

}
