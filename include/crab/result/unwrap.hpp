
#pragma once

#include "crab/core.hpp"
#include "crab/result/Result.hpp"

namespace crab {
  namespace result {

    template<typename T, typename E>
    [[nodiscard]] CRAB_INLINE constexpr auto unwrap(
      Result<T, E>&& result,
      const SourceLocation loc = SourceLocation::current()
    ) -> T {
      return mem::forward<Result<T, E>>(result).unwrap(loc);
    }

    template<typename T, typename E>
    [[nodiscard]] CRAB_INLINE constexpr auto unwrap_err(
      Result<T, E>&& result,
      const SourceLocation loc = SourceLocation::current()
    ) -> E {
      return mem::forward<Result<T, E>>(result).unwrap_err(loc);
    }
  }

  using result::unwrap;
  using result::unwrap_err;
}
