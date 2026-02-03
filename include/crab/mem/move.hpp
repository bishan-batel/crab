/// @file crab/mem/move.hpp

#pragma once

#include <utility>
#include "crab/core.hpp"

namespace crab::mem {
  /// @addtogroup mem
  /// @{

  /// Equivalent to `std::move` with the added constraint of not being able to move from const,
  /// which is almost always a bug.
  template<typename T>
  [[nodiscard]] CRAB_INLINE constexpr auto move(const T&& value) noexcept = delete;

  using std::move;
  /// }@
}

namespace crab {
  using crab::mem::move;
}
