#pragma once

#include "crab/preamble.hpp"
#include "crab/type_traits.hpp"

namespace crab::mem {
  /**
   * Crab version of sizeof(T) that disallows reference types, using reference types with sizeof is almost always a
   * bug
   */
  template<ty::non_reference T>
  CRAB_NODISCARD CRAB_CONSTEVAL auto size_of() -> usize {
    static_assert(ty::non_reference<T>, "Cannot get the sizeof a reference.");
    return sizeof(T);
  }

  /**
   * Returns the size of a given value's type
   */
  template<typename T>
  CRAB_NODISCARD CRAB_CONSTEVAL auto size_of_val(const T&) -> usize {
    return size_of<T>();
  }

}
