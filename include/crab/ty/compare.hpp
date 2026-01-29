/// @file compare.hpp
/// crab::ty helpers for comparisons between type, like 'same_as', 'different_than', 'either', etc

#pragma once

#include "crab/num/integer.hpp"

#include <concepts>
#include <tuple>

namespace crab::ty {
  /// 'indexes' into a variadic pack to get the nth type
  ///
  /// TODO: move nth_type to a more suitable header
  template<usize Index, typename... T>
  using nth_type = std::tuple_element_t<Index, std::tuple<T...>>;

  /// Requirement for the two given types to be exactly the same.
  template<typename A, typename B>
  concept same_as = std::same_as<A, B>;

  /// Requirement for all of the given types to be exactly the same.
  template<typename... T>
  concept all_same = sizeof...(T) < 2 or (same_as<nth_type<0, T...>, T> and ...);

  /// Requirement for the two given types to not be the same.
  template<typename A, typename B>
  concept different_than = not same_as<A, B>;

  /// Requirement for the given to be any one of the other types listed
  template<typename T, typename... Cases>
  concept either = (std::same_as<T, Cases> || ...);

}
