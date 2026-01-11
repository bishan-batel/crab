#pragma once

#include "size_of.hpp"
#include "crab/debug.hpp"
#include "crab/type_traits.hpp"

#include <cstring>

namespace crab::mem {

  template<ty::non_const T>
  auto copy(T* source, T* destination, const usize count) -> void {
    debug_assert(source != nullptr, "mem::copy cannot copy from nullptr");
    debug_assert(destination != nullptr, "mem::copy cannot copy to nullptr");

    // check for alignment
    debug_assert(static_cast<uptr>(source) % alignof(T) == 0, "mem::copy must copy from a correctly aligned pointer");
    debug_assert(
      static_cast<uptr>(destination) % alignof(T) == 0,
      "mem::copy must copy into a correctly aligned pointer"
    );

    static constexpr usize SIZE{mem::size_of<T>()};

    if constexpr (SIZE == 1) {
      memmove(static_cast<void*>(destination), static_cast<void*>(source), count);
    } else {
      // check for integer overflow
      debug_assert(
        SIZE == 0 or count > std::numeric_limits<usize>::max() / SIZE,
        "mem::copy integeroverflow converting count -> byte_count"
      );

      const usize byte_count{count * mem::size_of<T>()};

      memmove(static_cast<void*>(destination), static_cast<void*>(source), byte_count);
    }
  }

  template<ty::non_const T>
  auto copy_nonoverlapping(T* source, T* destination, const usize count) -> void {
    debug_assert(source != nullptr, "mem::copy_nonoverlapping cannot copy from nullptr");
    debug_assert(destination != nullptr, "mem::copy_nonoverlapping cannot copy to nullptr");

    // check for alignment
    debug_assert(
      static_cast<uptr>(source) % alignof(T) == 0,
      "mem::copy_nonoverlapping must copy from a correctly aligned pointer"
    );

    debug_assert(
      static_cast<uptr>(destination) % alignof(T) == 0,
      "mem::copy_nonoverlapping must copy into a correctly aligned pointer"
    );

    static constexpr usize SIZE{mem::size_of<T>()};

    {
      uptr source_begin{static_cast<uptr>(&source[0])};
      uptr source_end{static_cast<uptr>(&source[count - 1])};

      uptr dest_begin{static_cast<uptr>(&destination[0])};
      uptr dest_end{static_cast<uptr>(&destination[count - 1])};

      debug_assert(
        not(source_begin <= dest_end and dest_begin >= source_end),
        "mem::copy_nonoverlapping must operate on two non overlapping spans"
      );
    }

    // debug_assert();

    if constexpr (SIZE == 1) {
      memcpy(static_cast<void*>(destination), static_cast<void*>(source), count);
    } else {
      // check for integer overflow
      debug_assert(
        SIZE == 0 or count > std::numeric_limits<usize>::max() / SIZE,
        "mem::copy_nonoverlapping integeroverflow converting count -> byte_count"
      );

      const usize byte_count{count * mem::size_of<T>()};

      memcpy(static_cast<void*>(destination), static_cast<void*>(source), byte_count);
    }
  }
}
