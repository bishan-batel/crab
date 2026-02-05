/// @file crab/result/unwrap.hpp
/// @ingroup ref

#pragma once

#include "crab/core.hpp"
#include "crab/result/Result.hpp"

namespace crab {
  namespace result {

    /// Static method version of Result::unwrap. Exposed in the 'crab' namespace as well.
    ///
    /// @copydoc crab::result::Result::unwrap
    /// @ingroup ref
    template<typename T, typename E>
    [[nodiscard]] CRAB_INLINE constexpr auto unwrap(
      Result<T, E>&& result,
      const SourceLocation loc = SourceLocation::current()
    ) -> T {
      return mem::move<Result<T, E>>(result).unwrap(loc);
    }

    /// Static method version of Result::unwrap_err. Exposed in the 'crab' namespace as well.
    ///
    /// @copydoc crab::result::Result::unwrap_err
    /// @ingroup ref
    template<typename T, typename E>
    [[nodiscard]] CRAB_INLINE constexpr auto unwrap_err(
      Result<T, E>&& result,
      const SourceLocation loc = SourceLocation::current()
    ) -> E {
      return mem::move<Result<T, E>>(result).unwrap_err(loc);
    }
  }

  using result::unwrap;
  using result::unwrap_err;
}
