#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include "crab/debug.hpp"
#include "preamble.hpp"

template<std::integral T = usize>
class Range final {
  T min, max;

public:

  /**
   * Iterator type for Range
   */
  struct Iterator {
    using iterator_category = std::output_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T;
    using reference = T;

    inline constexpr explicit Iterator(T pos): pos(pos) {}

    inline constexpr auto operator*() const -> reference { return pos; }

    inline constexpr auto operator->() -> pointer { return pos; }

    inline constexpr auto operator++() -> Iterator& {
      ++pos;
      return *this;
    }

    inline constexpr auto operator++(int) -> Iterator {
      Iterator tmp = *this;
      ++*this;
      return tmp;
    }

    inline friend constexpr auto operator==(
      const Iterator& a,
      const Iterator& b
    ) -> bool {
      return a.pos == b.pos;
    };

    inline friend constexpr auto operator!=(
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
  inline constexpr Range(T min, T max): min(min), max(max) {
    debug_assert(min <= max, "Invalid Range, max cannot be greater than min");
  }

  /**
   * Returns the lower bound of this range
   */
  [[nodiscard]] inline constexpr auto upper_bound() const -> T { return max; }

  /**
   * Returns the lower bound of this range
   */
  [[nodiscard]] inline constexpr auto lower_bound() const -> T { return min; }

  /**
   * Begin iterator position.
   */
  [[nodiscard]] inline constexpr auto begin() const -> Iterator {
    return Iterator(min);
  }

  /**
   * End iterator position.
   */
  [[nodiscard]] inline constexpr auto end() const -> Iterator {
    return Iterator(max);
  }

  /**
   * Returns the length of this range
   */
  [[nodiscard]] inline constexpr auto size() const -> usize {
    return static_cast<usize>(max - min);
  }

  /**
   * @brief Checks if the given value is within this range
   */
  [[nodiscard]] inline constexpr auto contains(const T value) const -> bool {
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
  [[nodiscard]] inline constexpr auto range(
    std::type_identity_t<T> min,
    std::type_identity_t<T> max
  ) -> Range<T> {
    return Range<T>(min, max);
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
  inline constexpr auto range(std::type_identity_t<T> max) -> Range<T> {
    return range<T>(0, max);
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
  [[nodiscard]] inline constexpr auto range_inclusive(
    std::type_identity_t<T> min,
    std::type_identity_t<T> max
  ) -> Range<T> {
    return range(min, max + 1);
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
  [[nodiscard]] inline constexpr auto range_inclusive(
    std::type_identity_t<T> max
  ) -> Range<T> {
    return range(max + 1);
  }
} // namespace crab
