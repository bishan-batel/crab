#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>

#include <crab/preamble.hpp>
#include <crab/debug.hpp>
#include "crab/type_traits.hpp"

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

    constexpr explicit Iterator(T pos): pos(pos) {}

    [[nodiscard]] constexpr auto operator*() const -> reference { return pos; }

    [[nodiscard]] constexpr auto operator->() -> pointer { return pos; }

    constexpr auto operator++() -> Iterator& {
      ++pos;
      return *this;
    }

    [[nodiscard]] constexpr auto operator++(int) -> Iterator {
      Iterator tmp = *this;
      ++*this;
      return tmp;
    }

    [[nodiscard]] friend constexpr auto operator==(
      const Iterator& a,
      const Iterator& b
    ) -> bool {
      return a.pos == b.pos;
    };

    [[nodiscard]] friend constexpr auto operator!=(
      const Iterator& a,
      const Iterator& b
    ) -> bool {
      return a.pos != b.pos;
    };

  private:

    T pos;
  };

  /**
   * Constructs a range from min to max, this will panic if max > min
   */
  constexpr Range(
    T min,
    T max,
    const SourceLocation loc = SourceLocation::current()
  ):
      min(min), max(max) {
    debug_assert_transparent(
      min <= max,
      loc,
      "Invalid Range, max cannot be greater than min"
    );
  }

  /**
   * Returns the lower bound of this range
   */
  [[nodiscard]] constexpr auto upper_bound() const -> T { return max; }

  /**
   * Returns the lower bound of this range
   */
  [[nodiscard]] constexpr auto lower_bound() const -> T { return min; }

  /**
   * Begin iterator position.
   */
  [[nodiscard]] constexpr auto begin() const -> Iterator {
    return Iterator{min};
  }

  /**
   * End iterator position.
   */
  [[nodiscard]] constexpr auto end() const -> Iterator { return Iterator{max}; }

  /**
   * Returns the length of this range
   */
  [[nodiscard]] constexpr auto size() const -> usize {
    return static_cast<usize>(max - min);
  }

  /**
   * @brief Checks if the given value is within this range
   */
  [[nodiscard]] constexpr auto contains(const T value) const -> bool {
    return min <= value and value < max;
  }
};

namespace crab {
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
  [[nodiscard]] constexpr auto range(
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
  [[nodiscard]]
  constexpr auto range(
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
  [[nodiscard]] constexpr auto range_inclusive(
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
  [[nodiscard]] constexpr auto range_inclusive(
    std::type_identity_t<T> max,
    const SourceLocation loc = SourceLocation::current()
  ) -> Range<T> {
    return range(max + 1, loc);
  }
} // namespace crab
