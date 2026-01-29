/// @file preamble.hpp
/// Convenience header used by users of crab that imports common headers & alias.
/// Note that this header will NEVER be included by another crab header.

#pragma once

#include "crab/core.hpp"

#include <utility>
#include <array>
#include <span>
#include <ranges>

#include "crab/core/cases.hpp"
#include "crab/core/unit.hpp"
#include "crab/core/SourceLocation.hpp"

#include "crab/num/integer.hpp"
#include "crab/num/floating.hpp"
#include "crab/num/suffixes.hpp"

#include "crab/str/str.hpp"

#include "crab/fn/Func.hpp"

namespace crab {
  /// Alias for std::pair.
  template<typename A, typename B>
  using Pair = std::pair<A, B>;

  /// Alias for std::array.
  /// Statically Sized list of packed objects
  template<typename T, usize length>
  using SizedArray = std::array<T, length>;

  /// Alias for std::span.
  /// This is an abstraction over any contiguous sequence of elements / slice.
  template<typename T, usize length = std::dynamic_extent>
  using Span = std::span<T, length>;

}

namespace crab::prelude {

  using ::crab::Pair;

#if !CRAB_NO_TYPEDEF_ARRAY
  using ::crab::SizedArray;
#endif

#if !CRAB_NO_TYPEDEF_SPAN
  using ::crab::Span;
#endif

  /// std::ranges Alias
  namespace ranges = ::std::ranges;

  /// std::ranges::views Alias
  namespace views = ::std::ranges::views;
}

CRAB_PRELUDE_GUARD;
