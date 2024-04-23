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

    constexpr explicit Iterator(T pos) : pos(pos) {}

    constexpr reference operator*() const { return pos; }

    constexpr pointer operator->() { return pos; }

    constexpr Iterator &operator++() {
      ++pos;
      return *this;
    }

    constexpr Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    constexpr friend bool operator==(const Iterator &a, const Iterator &b) {
      return a.pos == b.pos;
    };

    constexpr friend bool operator!=(const Iterator &a, const Iterator &b) {
      return a.pos != b.pos;
    };

  private:
    T pos;
  };

  constexpr Range(T min, T max)
    : min(min), max(max) {
    assert(min <= max and "Invalid Range, max cannot be greater than min");
  }

  [[nodiscard]] constexpr T upper_bound() const { return max; }

  [[nodiscard]] constexpr T lower_bound() const { return min; }

  [[nodiscard]] constexpr Iterator begin() const { return Iterator(min); }

  [[nodiscard]] constexpr Iterator end() const { return Iterator(max); }
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
  [[nodiscard]] Range<T> range(T min, T max) {
    return Range(min, max);
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
  [[nodiscard]] Range<T> range(T max) {
    return Range(0, max);
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
  [[nodiscard]] Range<T> range_inclusive(T min, T max) {
    return range(min, max + 1);
  }

  /**
   * Range from 0 to max (inclusive)
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
  [[nodiscard]] Range<T> range_inclusive(T max) {
    return range(max + 1);
  }
}
