#pragma once

#include <type_traits>
#include "crab/boxed/Box.hpp"
#include "crab/assertion/check.hpp"
#include "crab/mem/size_of.hpp"

#include <cstring>

/// @addtogroup mem
/// @{

namespace crab::mem {

  /// Perform a *bitwise* copy of *count* items from source to destination.
  /// The given pointers may be overlapping in ranges.
  ///
  /// @param source Pointer to start of the 'source' array to read from
  /// @param destination Pointer to the start of the 'destination' array to write to
  /// @param count The amount of T's in the array 'source' & to in the destination to copy to.
  ///
  /// # Panics
  /// This function will panic if sizeof(T) != 1 and sizeof(T) * count overflows.
  ///
  /// This function may also panic *only in debug mode* if any precondition is violated (see the Safety section).
  ///
  /// # Safety
  /// This function is unsafe because this will perform a *bitwise copy* from 'destination' to source, regardless if T
  /// is trivially copyable. This function has the following preconditions, if these are not met then behavior is
  /// undefined:
  /// - source must be valid to read `sizeof(T) * count` bytes
  /// - destination must be valid to read `sizeof(T) * count` bytes
  /// - both source and destination must be non-null (*even if count=0*)
  /// - both source and destination must be properly aligned (*even if count=0*)
  /// - the storage class for 'destination' must not have another subobject in its tail padding.
  template<ty::non_const T = u8>
  auto copy(unsafe_fn, const T* source, T* destination, const usize count) -> void {
    // null checks
    crab_dbg_check(source != nullptr and destination != nullptr, "mem::copy cannot copy from or to nullptr");

    // source msut be aligned
    crab_dbg_check(

      static_cast<uptr>(source) % alignof(T) == 0,
      "mem::copy must copy from a correctly aligned pointer"
    );

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
      crab_check(

        SIZE == 0 or count > std::numeric_limits<usize>::max() / SIZE,
        "mem::copy integeroverflow converting count -> byte_count"
      );

      const usize byte_count{count * mem::size_of<T>()};

      memmove(static_cast<void*>(destination), static_cast<void*>(source), byte_count);
    }
  }

  /// Perform a *bitwise* copy of *count* items from source to destination with ranges that *do not overlap*.
  ///
  /// @param source Pointer to start of the 'source' array to read from
  /// @param destination Pointer to the start of the 'destination' array to write to
  /// @param count The amount of T's in the array 'source' & to in the destination to copy to.
  ///
  /// # Panics
  /// This function will panic if sizeof(T) != 1 and sizeof(T) * count overflows.
  ///
  /// This function may also panic *only in debug mode* if any precondition is violated (see the Safety section).
  ///
  /// # Safety
  /// This function is unsafe because this will perform a *bitwise copy* from 'destination' to source, regardless if T
  /// is trivially copyable. This function has the following preconditions, if these are not met then behavior is
  /// undefined:
  /// - the range of source to `&source[count]` must not overlap with the range destination to `&destination[count]`
  /// - source must be valid to read `sizeof(T) * count` bytes
  /// - destination must be valid to read `sizeof(T) * count` bytes
  /// - both source and destination must be non-null (*even if count=0*)
  /// - both source and destination must be properly aligned (*even if count=0*)
  /// - the storage class for 'destination' must not have another subobject in its tail padding.
  template<ty::non_const T = u8>
  auto copy_nonoverlapping(unsafe_fn, const T* source, T* destination, const usize count) -> void {
    crab_dbg_check(

      source != nullptr and destination != nullptr,
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

    if constexpr (SIZE == 1) {
      memcpy(static_cast<void*>(destination), static_cast<void*>(source), count);
    } else {
      // check for integer overflow
      crab_check(

        SIZE == 0 or count > std::numeric_limits<usize>::max() / SIZE,
        "mem::copy_nonoverlapping integeroverflow converting count -> byte_count"
      );

      const usize byte_count{count * mem::size_of<T>()};

      memcpy(static_cast<void*>(destination), static_cast<void*>(source), byte_count);
    }
  }

}

/// }@
