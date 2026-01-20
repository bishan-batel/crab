#pragma once

#include <cassert>
#include <iterator>

#include "crab/core.hpp"
#include "crab/num/integer.hpp"
#include "crab/type_traits.hpp"
#include "crab/assertion/assert.hpp"

namespace crab::range {
  template<std::integral T = usize>
  class Range final {
    T min, max;

  public:

    /**
     * Iterator type for Range
     */
    struct Iterator {
      using iterator_category = std::output_iterator_tag;
      using difference_type = ptrdiff;
      using value_type = T;
      using pointer = T;
      using reference = T;

      CRAB_INLINE constexpr explicit Iterator(T pos): pos(pos) {}

      [[nodiscard]] CRAB_PURE CRAB_INLINE constexpr auto operator*() const -> reference {
        return pos;
      }

      [[nodiscard]] CRAB_PURE CRAB_INLINE constexpr auto operator->() -> pointer {
        return pos;
      }

      CRAB_PURE CRAB_INLINE constexpr auto operator++() -> Iterator& {
        ++pos;
        return *this;
      }

      [[nodiscard]] CRAB_PURE CRAB_INLINE constexpr auto operator++(int) -> Iterator {
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

      T pos;
    };

    /**
     * Constructs a range from min to max, this will panic if max > min
     */
    CRAB_INLINE constexpr Range(T min, T max, const SourceLocation loc = SourceLocation::current()):
        min(min), max(max) {
      debug_assert_transparent(min <= max, loc, "Invalid Range, max cannot be greater than min");
    }

    /**
     * Returns the lower bound of this range
     */
    [[nodiscard]] CRAB_PURE CRAB_INLINE constexpr auto upper_bound() const -> T {
      return max;
    }

    /**
     * Returns the lower bound of this range
     */
    [[nodiscard]] CRAB_PURE constexpr auto lower_bound() const -> T {
      return min;
    }

    /**
     * Begin iterator position.
     */
    [[nodiscard]] CRAB_PURE constexpr auto begin() const -> Iterator {
      return Iterator{min};
    }

    /**
     * End iterator position.
     */
    [[nodiscard]] CRAB_PURE constexpr auto end() const -> Iterator {
      return Iterator{max};
    }

    /**
     * Returns the length of this range
     */
    [[nodiscard]] CRAB_PURE constexpr auto size() const -> usize {
      return static_cast<usize>(max - min);
    }

    /**
     * @brief Checks if the given value is within this range
     */
    [[nodiscard]] CRAB_PURE constexpr auto contains(const T value) const -> bool {
      return min <= value and value < max;
    }
  };

  /**
   * Range from min to max (exclusive)
   *
   * doing
   *
   * for (usize i : range(5, 100))
   *
   * is the same as
   *
   * for (usize i = 5; i < 100; i++)
   */
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
    return range<T>(0, max, loc);
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
    return range(min, max + 1, loc);
  }

  /**
   * @brief Range from 0 to max (inclusive)
   *
   * doing
   *
   * for (usize i : range(100))
   *
   * is the same as
   *
   * for (usize i = 0; i <= 100; i++)
   */
  template<std::integral T = usize>
  [[nodiscard]] CRAB_INLINE constexpr auto range_inclusive(
    std::type_identity_t<T> max,
    const SourceLocation loc = SourceLocation::current()
  ) -> Range<T> {
    return range(max + 1, loc);
  }

} // namespace crab

namespace crab::prelude {
  using crab::range::Range;
}

CRAB_PRELUDE_GUARD;
