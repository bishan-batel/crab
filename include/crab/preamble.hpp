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
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "crab/core/cases.hpp"
#include "crab/core/unit.hpp"

#include "crab/num/integer.hpp"
#include "crab/num/floating.hpp"
#include "crab/num/suffixes.hpp"

#include "crab/str/str.hpp"

namespace crab {
  using SourceLocation = std::source_location;

  /**
   * @brief Function pointer that supports lambdas with captures
   */
  template<typename F = void()>
  using Func = std::function<F>;

  /**
   * @brief std::tuple<T...> alias.
   */
  template<typename... Types>
  using Tuple = std::tuple<Types...>;

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

  /**
   * @brief Heap allocated, dynamically sized list
   */
  template<typename T>
  using Vec = std::vector<T>;

  /**
   * @brief Unordered set of elements
   */
  template<typename T, typename Hash = std::hash<T>, typename Predicate = std::equal_to<T>>
  using Set = std::unordered_set<T, Hash, Predicate>;

  /**
   * @brief Unordered key-value collection
   */
  template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Predicate = std::equal_to<Key>>
  using Dictionary = std::unordered_map<Key, Value, Hash, Predicate>;

}

namespace crab::assertion {
  CRAB_NORETURN auto panic(StringView msg, SourceLocation loc) -> void;
}

namespace crab::prelude {

  using ::crab::Func;
  using ::crab::Tuple;
  using ::crab::Pair;

#if !CRAB_NO_TYPEDEF_ARRAY
  using ::crab::SizedArray;
#endif

#if !CRAB_NO_TYPEDEF_SPAN
  using ::crab::Span;
#endif

#if !CRAB_NO_TYPEDEF_VEC
  using ::crab::Vec;
#endif

#if !CRAB_NO_TYPEDEF_SET
  using ::crab::Set;
#endif

#if !CRAB_NO_TYPEDEF_DICTIONARY
  using ::crab::Dictionary;
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
