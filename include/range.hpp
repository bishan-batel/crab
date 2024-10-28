#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include "preamble.hpp"

template<std::integral T = usize>
class Range final {
  T min, max;

public:
  struct Iterator {
    using iterator_category = std::output_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T;
    using reference = T;

    constexpr explicit Iterator(T pos) : pos(pos) {}

    constexpr auto operator*() const -> reference { return pos; }

    constexpr auto operator->() -> pointer { return pos; }

    constexpr auto operator++() -> Iterator & {
      ++pos;
      return *this;
    }

    constexpr auto operator++(int) -> Iterator {
      Iterator tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr friend auto operator<=>(const Iterator &a, const Iterator &b) -> bool { return b.pos - a.pos; }

  private:
    T pos;
  };

  Range(T min, T max) : min(min), max(max) { assert(min <= max and "Invalid Range, max cannot be greater than min"); }

  [[nodiscard]] constexpr auto upper_bound() const -> T { return max; }

  [[nodiscard]] constexpr auto lower_bound() const -> T { return min; }

  [[nodiscard]] constexpr auto begin() const -> Iterator { return Iterator(min); }

  [[nodiscard]] constexpr auto end() const -> Iterator { return Iterator(max); }
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
  [[nodiscard]] constexpr auto range(T min, T max) -> Range<T> {
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
  [[nodiscard]] constexpr auto range(T max) -> Range<T> {
    return Range<T>(0, max);
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
  [[nodiscard]] constexpr auto range_inclusive(T min, T max) -> Range<T> {
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
  [[nodiscard]] constexpr auto range_inclusive(T max) -> Range<T> {
    return range(max + 1);
  }
} // namespace crab
