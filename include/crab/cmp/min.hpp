#pragma once

#include <ranges>
#include "crab/ty/functor.hpp"

namespace crab::cmp {

  template<typename T, ty::predicate<T, T> Comparator = std::less<T>>
  [[nodiscard]] constexpr auto min(T&& lhs, T&& rhs, Comparator&& comparator = std::less<T>{}) -> T {
    return std::invoke(comparator, lhs, rhs) ? lhs : rhs;
  }

  template<std::ranges::input_range R, typename Comparator = std::less<std::ranges::range_value_t<R>>>
  [[nodiscard]] constexpr auto min_element(
    R&& range,
    Comparator&& comparator = std::less<std::ranges::range_value_t<R>>{}
  ) -> std::ranges::iterator_t<R> {
    namespace ranges = std::ranges;

    using Iter = ranges::iterator_t<R>;

    Iter iter{ranges::begin(range)};
    Iter end{ranges::end(range)};

    if (iter == end) {
      return iter;
    }

    Iter minimum{iter};

    for (; iter != end; ++iter) {
      if (comparator(*iter, *minimum)) {
        minimum = iter;
      }
    }

    return minimum;
  }
}
