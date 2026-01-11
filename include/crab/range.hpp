#pragma once

#include <cassert>
#include <iterator>

#include "crab/num/integer.hpp"
#include "crab/type_traits.hpp"
#include "crab/assertion/assert.hpp"

namespace crab {
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

      CRAB_INLINE_CONSTEXPR explicit Iterator(T pos): pos(pos) {}

      CRAB_NODISCARD_INLINE_CONSTEXPR auto operator*() const -> reference {
        return pos;
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR auto operator->() -> pointer {
        return pos;
      }

      CRAB_INLINE_CONSTEXPR auto operator++() -> Iterator& {
        ++pos;
        return *this;
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR auto operator++(int) -> Iterator {
        Iterator tmp = *this;
        ++*this;
        return tmp;
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR friend auto operator==(const Iterator& a, const Iterator& b) -> bool {
        return a.pos == b.pos;
      };

      CRAB_NODISCARD_INLINE_CONSTEXPR friend auto operator!=(const Iterator& a, const Iterator& b) -> bool {
        return a.pos != b.pos;
      };

    private:

      T pos;
    };

    /**
     * Constructs a range from min to max, this will panic if max > min
     */
    CRAB_INLINE_CONSTEXPR Range(T min, T max, const SourceLocation loc = SourceLocation::current()):
        min(min), max(max) {
      debug_assert_transparent(min <= max, loc, "Invalid Range, max cannot be greater than min");
    }

    /**
     * Returns the lower bound of this range
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto upper_bound() const -> T {
      return max;
    }

    /**
     * Returns the lower bound of this range
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto lower_bound() const -> T {
      return min;
    }

    /**
     * Begin iterator position.
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto begin() const -> Iterator {
      return Iterator{min};
    }

    /**
     * End iterator position.
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto end() const -> Iterator {
      return Iterator{max};
    }

    /**
     * Returns the length of this range
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto size() const -> usize {
      return static_cast<usize>(max - min);
    }

    /**
     * @brief Checks if the given value is within this range
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto contains(const T value) const -> bool {
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
  CRAB_NODISCARD_INLINE_CONSTEXPR auto range(
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
  CRAB_NODISCARD_INLINE_CONSTEXPR auto range(
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
  CRAB_NODISCARD_INLINE_CONSTEXPR auto range_inclusive(
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
  CRAB_NODISCARD_INLINE_CONSTEXPR auto range_inclusive(
    std::type_identity_t<T> max,
    const SourceLocation loc = SourceLocation::current()
  ) -> Range<T> {
    return range(max + 1, loc);
  }
} // namespace crab

#if CRAB_USE_PRELUDE

using crab::Range;

#endif
