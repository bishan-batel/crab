#pragma once

#include "crab/assertion/check.hpp"
#include "size_of.hpp"

#include <cstring>

namespace crab::mem {

  template<ty::non_const T>
  auto copy(const T* source, T* destination, const usize count) -> void {
    // null checks
    crab_dbg_check(source != nullptr && destination != nullptr, "mem::copy cannot copy from or to nullptr");

    // source msut be aligned
    crab_dbg_check(static_cast<uptr>(source) % alignof(T) == 0, "mem::copy must copy from a correctly aligned pointer");

    // dest must be aligned
    crab_dbg_check(
      static_cast<uptr>(destination) % alignof(T) == 0,
      "mem::copy must copy into a correctly aligned pointer"
    );

    static constexpr usize SIZE{mem::size_of<T>()};

    if constexpr (SIZE == 1) {
      memmove(static_cast<void*>(destination), static_cast<void*>(source), count);
    } else {
      // check for integer overflow when multiplying
      crab_dbg_check(
        SIZE == 0 or count > std::numeric_limits<usize>::max() / SIZE,
        "mem::copy integeroverflow converting count -> byte_count"
      );

      const usize byte_count{count * mem::size_of<T>()};

      memmove(static_cast<void*>(destination), static_cast<void*>(source), byte_count);
    }
  }

  template<ty::non_const T>
  auto copy_nonoverlapping(const T* source, T* destination, const usize count) -> void {
    crab_dbg_check(
      source != nullptr && destination != nullptr,
      "mem::copy_nonoverlapping cannot copy from or to nullptr"
    );

    // check for alignment
    crab_dbg_check(
      static_cast<uptr>(source) % alignof(T) == 0,
      "mem::copy_nonoverlapping must copy from a correctly aligned pointer"
    );

    crab_dbg_check(
      static_cast<uptr>(destination) % alignof(T) == 0,
      "mem::copy_nonoverlapping must copy into a correctly aligned pointer"
    );

    static constexpr usize SIZE{mem::size_of<T>()};

    // double check that the ranges dont overlap
    {
      const uptr source_begin{static_cast<uptr>(&source[0])};
      const uptr source_end{static_cast<uptr>(&source[count - 1])};

      const uptr dest_begin{static_cast<uptr>(&destination[0])};
      const uptr dest_end{static_cast<uptr>(&destination[count - 1])};

      crab_dbg_check(
        not(source_begin <= dest_end and dest_begin >= source_end),
        "mem::copy_nonoverlapping must operate on two non overlapping spans"
      );
    }

    // crab_check();

    if constexpr (SIZE == 1) {
      memcpy(static_cast<void*>(destination), static_cast<void*>(source), count);
    } else {
      // check for integer overflow
      crab_dbg_check(
        SIZE == 0 or count > std::numeric_limits<usize>::max() / SIZE,
        "mem::copy_nonoverlapping integeroverflow converting count -> byte_count"
      );

      const usize byte_count{count * mem::size_of<T>()};

      memcpy(static_cast<void*>(destination), static_cast<void*>(source), byte_count);
    }
  }
}
