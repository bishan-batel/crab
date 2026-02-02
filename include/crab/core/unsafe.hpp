/// @file crab/core/unsafe.hpp

#include <array>
#include <bit>
#include "crab/num/integer.hpp"

namespace crab {

  /// Unsafe marker type, this type is used
  /// This type should appear as an unamed parameter
  /// @ingroup core
  struct unsafe_fn final {};

  /// Constant  meant to be passed to functions marked as 'unsafe' with the tag 'unsafe_t'.
  /// @ingroup core
  inline static constexpr unsafe_fn unsafe{};

}

namespace crab::prelude {
  using crab::unsafe_fn;
  using crab::unsafe;
}
