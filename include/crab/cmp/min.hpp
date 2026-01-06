#pragma once

#include <initializer_list>
#include <ranges>
#include "crab/preamble.hpp"
#include "crab/type_traits.hpp"

namespace crab::cmp {

  template<typename T, ty::predicate<T, T> Comparator = std::less<T>>
  CRAB_PURE_INLINE_CONSTEXPR auto min(T&& lhs, T&& rhs, Comparator&& comparator = std::less<T>{}) -> T {
    return std::invoke(comparator, lhs, rhs) ? lhs : rhs;
  }

  template<std::ranges::input_range R, typename Comparator = std::less<std::ranges::range_value_t<R>>>
  CRAB_PURE_INLINE_CONSTEXPR auto min_element(
    R&& range,
    Comparator&& comparator = std::less<std::ranges::range_value_t<R>>{}
  ) -> ranges::iterator_t<R> {
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
