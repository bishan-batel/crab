/**
 * Created by Kishan Patel (bishan.batel@protonmail.com) on 3/3/2024
 */

#pragma once

#include "./core.hpp"

#include <functional>
#include <source_location>
#include <sstream>
#include <string>
#include <utility>
#include <array>
#include <span>
#include <ranges>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "crab/core/cases.hpp"
#include "crab/core/unit.hpp"
#include "crab/core/SourceLocation.hpp"

#include "crab/num/integer.hpp"
#include "crab/num/floating.hpp"
#include "crab/num/suffixes.hpp"

#include "crab/str/str.hpp"

#include "crab/fn/Func.hpp"

namespace crab {
  /**
   * @brief std::pair<T, S> alias.
   */
  template<typename A, typename B>
  using Pair = std::pair<A, B>;

  /**
   * @brief Alias for std::array
   *
   * Statically Sized list of packed objects
   *
   * @tparam T
   */
  template<typename T, usize length>
  using SizedArray = std::array<T, length>;

  /**
   * @brief Abstraction over any contiguous sequence of elements
   */
  template<typename T, usize length = std::dynamic_extent>
  using Span = std::span<T, length>;

}

namespace crab::prelude {

  using ::crab::Func;
  using ::crab::Pair;

#if !CRAB_NO_TYPEDEF_ARRAY
  using ::crab::SizedArray;
#endif

#if !CRAB_NO_TYPEDEF_SPAN
  using ::crab::Span;
#endif

  /**
   * std::ranges
   */
  namespace ranges = ::std::ranges;

  /**
   * std::ranges::views
   */
  namespace views = ::std::ranges::views;
}

CRAB_PRELUDE_GUARD;
