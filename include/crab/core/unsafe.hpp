/// @file

#include <array>
#include <bit>
#include "crab/num/integer.hpp"

namespace crab {

  /// Unsafe marker type, this type is used
  /// This type should appear as an unamed parameter
  struct unsafe_fn final {
    unsafe_fn() = delete;
  };

  /// Constant  meant to be passed to functions marked as 'unsafe' with the tag 'unsafe_t'.
  static constexpr unsafe_fn unsafe{
    std::bit_cast<unsafe_fn>(std::array<u8, sizeof(unsafe_fn)>{}),
  };

}

namespace crab::prelude {
  using crab::unsafe_fn;
  using crab::unsafe;
}
