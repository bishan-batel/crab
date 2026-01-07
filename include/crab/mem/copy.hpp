#pragma once

#include "size_of.hpp"
#include "crab/debug.hpp"
#include "crab/type_traits.hpp"

#include <cstring>

namespace crab::mem {

  template<ty::non_const T>
  auto copy(T* source, T* destination, const usize count, const SourceLocation& loc = SourceLocation::current()) -> void {
    debug_assert_transparent(source != nullptr, loc, "mem::copy cannot copy from nullptr");
    debug_assert_transparent(destination != nullptr, loc, "mem::copy cannot copy to nullptr");

    // check for alignment
    debug_assert_transparent(static_cast<uptr>(source) % alignof(T) == 0, loc, "mem::copy must copy from a correctly aligned pointer");
    debug_assert_transparent(static_cast<uptr>(destination) % alignof(T) == 0, loc, "mem::copy must copy into a correctly aligned pointer");

    static constexpr usize SIZE{mem::size_of<T>()};

    if constexpr(SIZE == 1) {
      memmove(static_cast<void*>(destination),static_cast<void*>(source), count);
    } else {
      // check for integer overflow
      debug_assert_transparent(SIZE == 0 or count  > std::numeric_limits<usize>::max() / SIZE, loc, "mem::copy integeroverflow converting count -> byte_count");

      const usize byte_count{count * mem::size_of<T>()};

      memmove(static_cast<void*>(destination),static_cast<void*>(source), byte_count);
    }
  }
}
