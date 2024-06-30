#pragma once

#include "preamble.hpp"
#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>

template<typename T = usize>
  requires std::is_integral_v<T>
class Range final {
  T min, max;

public:
  struct Iterator {
    using iterator_category = std::output_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T;
    using reference = T;

    __always_inline constexpr explicit Iterator(T pos) : pos(pos) {}

    __always_inline constexpr auto operator*() const -> reference { return pos; }

    __always_inline constexpr auto operator->() -> pointer { return pos; }

    __always_inline constexpr auto operator++() -> Iterator& {
      ++pos;
      return *this;
    }

    __always_inline constexpr auto operator++(int) -> Iterator {
      Iterator tmp = *this;
      ++*this;
      return tmp;
    }

    __always_inline constexpr friend auto operator==(const Iterator &a, const Iterator &b) -> bool {
      return a.pos == b.pos;
    };

    __always_inline constexpr friend auto operator!=(const Iterator &a, const Iterator &b) -> bool {
      return a.pos != b.pos;
    };

  private:
    T pos;
  };

  Range(T min, T max)
    : min(min), max(max) {
    assert(min <= max and "Invalid Range, max cannot be greater than min");
  }

  [[nodiscard]] __always_inline constexpr auto upper_bound() const -> T { return max; }

  [[nodiscard]] __always_inline constexpr auto lower_bound() const -> T { return min; }

  [[nodiscard]] __always_inline constexpr auto begin() const -> Iterator { return Iterator(min); }

  [[nodiscard]] __always_inline constexpr auto end() const -> Iterator { return Iterator(max); }
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
  template<typename T>
    requires std::is_integral_v<T>
  [[nodiscard]]__always_inline constexpr auto range(T min, T max) -> Range<T> {
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
  template<typename T>
    requires std::is_integral_v<T>
  [[nodiscard]]__always_inline constexpr auto range(T max) -> Range<T> {
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
  template<typename T>
    requires std::is_integral_v<T>
  [[nodiscard]]__always_inline constexpr auto range_inclusive(T min, T max) -> Range<T> {
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
  template<typename T>
    requires std::is_integral_v<T>
  [[nodiscard]]__always_inline constexpr auto range_inclusive(T max) -> Range<T> {
    return range(max + 1);
  }
}
