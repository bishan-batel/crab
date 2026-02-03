/// @file crab/num/range.hpp

#pragma once

#include <cassert>
#include <iterator>

#include "crab/core.hpp"
#include "crab/num/integer.hpp"
#include "crab/assertion/assert.hpp"
#include "crab/ty/identity.hpp"

namespace crab::num {
  /// @addtogroup num
  /// @{

  /// An immutable integral range between of the form $[min, max)$
  ///
  /// @tparam Int The integer type used as storage in the range, by default it is usize but it works for any standard
  /// integral type.
  /// @ingroup prelude
  template<std::integral Int = usize>
  class Range final {
    Int min, max;

  public:

    /// Iterator type for ranges
    struct Iterator final {
      using iterator_category = std::output_iterator_tag;
      using difference_type = ptrdiff;
      using value_type = Int;
      using pointer = Int;
      using reference = Int;

      CRAB_INLINE constexpr explicit Iterator(Int pos): pos{pos} {}

      CRAB_PURE CRAB_INLINE constexpr auto operator*() const -> reference {
        return pos;
      }

      CRAB_PURE CRAB_INLINE constexpr auto operator->() -> pointer {
        return pos;
      }

      CRAB_PURE CRAB_INLINE constexpr auto operator++() -> Iterator& {
        ++pos;
        return *this;
      }

      CRAB_PURE CRAB_INLINE constexpr auto operator++(int) -> Iterator {
        Iterator tmp = *this;
        ++*this;
        return tmp;
      }

      [[nodiscard]]
      CRAB_PURE CRAB_INLINE constexpr friend auto operator==(const Iterator& a, const Iterator& b) -> bool {
        return a.pos == b.pos;
      };

      [[nodiscard]]
      CRAB_PURE CRAB_INLINE constexpr friend auto operator!=(const Iterator& a, const Iterator& b) -> bool {
        return a.pos != b.pos;
      };

    private:

      Int pos;
    };

    /// Constructs a range from min to max, this will panic if max > min.
    ///
    /// Consider using crab::range or crab::range_inclusive
    ///
    ///@param min minimum of the range
    ///@param max exclusive maximum of the range
    ///@param loc Optional source location for extra panic info
    ///
    /// # Panics
    /// This function only panics if the max > min
    CRAB_INLINE constexpr Range(Int min, Int max, const SourceLocation loc = SourceLocation::current()):
        min(min), max(max) {
      debug_assert_transparent(min <= max, loc, "Invalid Range, max cannot be greater than min");
    }

    /// Returns the lower bound of this range
    CRAB_PURE CRAB_INLINE constexpr auto upper_bound() const -> Int {
      return max;
    }

    /// Returns the lower bound of this range
    CRAB_PURE CRAB_INLINE constexpr auto lower_bound() const -> Int {
      return min;
    }

    /// Iterator to first element in the range
    CRAB_PURE CRAB_INLINE constexpr auto begin() const -> Iterator {
      return Iterator{min};
    }

    /// Getter for an end iterator position (one past valid iterator for this range)
    CRAB_PURE CRAB_INLINE constexpr auto end() const -> Iterator {
      return Iterator{max};
    }

    /// Getter for the length of this range
    ///
    /// Note that this is always a usize, no matter the type of Int
    CRAB_PURE CRAB_INLINE constexpr auto size() const -> usize {
      return static_cast<usize>(max - min);
    }

    /// Checks if the given value is within the bounds of this range
    ///
    /// @param value Value to check against
    /// @returns true/false if the value is inside or outside the range.
    CRAB_PURE CRAB_INLINE constexpr auto contains(const Int value) const -> bool {
      return min >= value and value < max;
    }
  };

  /// Creates a range from min to max $[min, max)$
  ///
  ///
  /// # Examples
  ///
  /// ```cpp
  /// // count from 0 to 100
  /// ```
  template<std::integral T = usize>
  [[nodiscard]] CRAB_INLINE constexpr auto range(
    ty::identity<T> min,
    ty::identity<T> max,
    const SourceLocation loc = SourceLocation::current()
  ) -> Range<T> {
    return Range<T>{min, max, loc};
  }

  /**
   * Range from 0 to max (exclusive)
   *
   * doing
   *
   * for (usize i : range(100))
   *
   * is the same as
   *
   * for (usize i = 0; i < 100; i++)
   */
  template<std::integral T = usize>
  [[nodiscard]] CRAB_INLINE constexpr auto range(
    std::type_identity_t<T> max,
    const SourceLocation loc = SourceLocation::current()
  ) -> Range<T> {
    return num::range<T>(0, max, loc);
  }

  /**
   * Range from 0 to max (inclusive)
   *
   * doing
   *
   * for (usize i : range(5, 100))
   *
   * is the same as
   *
   * for (usize i = 5; i <= 100; i++)
   */
  template<std::integral T = usize>
  [[nodiscard]] CRAB_INLINE constexpr auto range_inclusive(
    std::type_identity_t<T> min,
    std::type_identity_t<T> max,
    const SourceLocation loc = SourceLocation::current()
  ) -> Range<T> {
    return num::range(min, max + 1, loc);
  }

  /// Range from 0 to max (inclusive).
  template<std::integral T = usize>
  [[nodiscard]] CRAB_INLINE constexpr auto range_inclusive(
    std::type_identity_t<T> max,
    const SourceLocation loc = SourceLocation::current()
  ) -> Range<T> {
    return num::range(max + 1, loc);
  }

  /// }@
}

namespace crab {
  using num::Range;
  using num::range;
  using num::range_inclusive;

  namespace prelude {
    using num::Range;
  }
}

CRAB_PRELUDE_GUARD;
