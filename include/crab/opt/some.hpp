#pragma once

#include <type_traits>
#include "crab/opt/Option.hpp"
#include "crab/mem/move.hpp"
#include "crab/mem/forward.hpp"

namespace crab::opt {

  /**
   * Creates an Option<T> from some value T
   */
  template<typename T>
  [[nodiscard]] CRAB_INLINE constexpr auto some(ty::identity<T>&& from) {
    return Option<T>{mem::forward<T>(from)};
  }

  /**
   * Creates an Option<T> from some value T
   */
  [[nodiscard]] CRAB_INLINE constexpr auto some(auto from) {
    return Option<std::remove_cvref_t<decltype(from)>>{mem::move(from)};
  }
}

namespace crab {
  using opt::some;
}
