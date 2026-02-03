/// @file crab/opt/none.hpp
/// @ingroup opt

#pragma once

#include "crab/core.hpp"

namespace crab::opt {
  /// '0-sized' struct to give into Option<T> to create an empty Option. This struct is not actually 0 sized due to the
  /// limitations of C++ and its object model. This type is really used as a 'tag' type for `crab::opt::Option`.
  /// @ingroup opt
  struct [[maybe_unused]] None final {

    /// Equality comparison for none, None is a unit type therefore this will
    /// always return true.
    CRAB_PURE CRAB_INLINE constexpr auto operator==(const None&) const -> bool {
      return true;
    }
  };

  /// 'None' value type for use with Option<T>.
  /// This symbol is also explode simply as `crab::none`
  /// @ingroup opt
  inline static constexpr None none{};

}

namespace crab {
  using crab::opt::none;
}
