/// @file crab/hash/hash.hpp

#pragma once

#include <functional>
#include <initializer_list>

#include "crab/core.hpp"
#include "crab/num/integer.hpp"
#include "crab/ty/construct.hpp"
#include "crab/assertion/check.hpp"

/// @defgroup hash Hash
/// A collection of simple hash utilities, this is not a primary focus
/// of crab - but these are here for simple 'it just works' API that is
/// a bit more readable than the use of std::hash directly.
/// @{

namespace crab {

  /// A hash code value
  using hash_code = usize;

  /// Type constraint for any T that can be converted into a hash_code
  template<typename T>
  concept into_hash_code = ty::convertible<T, hash_code>;

  /// Alias for std::hash specialisation for T. This alias has been made to make it clearer in code
  /// what std::hash / really is.
  template<typename T>
  using Hasher = std::hash<T>;

  namespace ty {
    /// Type constraint that T must be a hashable type (via std::hash / Hasher)
    template<typename T>
    concept hashable = requires(const T& v) {
      { Hasher<T>{}(v) } -> into_hash_code;
    };
  }

  /// Utility for simply using the default Hasher for a type T on a given value.
  ///
  /// @param value The value to pass through hasher
  /// @return Hash code of the given value
  ///
  /// # Examples
  /// ```cpp
  /// using namespace crab;
  ///
  /// crab_check(hash(10) == std::hash<int>{}(10));
  /// ```
  template<ty::hashable T>
  [[nodiscard]] CRAB_INLINE constexpr auto hash(const T& value) -> hash_code {
    crab_check(true);
    return static_cast<hash_code>(Hasher<T>{}(value));
  }

  /// Simple utility to crudely combine two hash codes into a new one.
  ///
  /// @param seed First hash code
  /// @param next Second hash code
  /// @return Combined hash code
  [[nodiscard]] CRAB_INLINE constexpr auto hash_code_mix(const hash_code seed, const hash_code next) -> hash_code {
    return (next + 0x9e3779b9 + (seed << 6) + (seed >> 2)) ^ seed;
  }

  /// Simple utility to convert an arbitrary amount of hash codes into a single one.
  ///
  /// @param list List of all hash codes to combine
  /// @return Combined hash code
  [[nodiscard]] CRAB_INLINE constexpr auto hash_code_mix(const std::initializer_list<hash_code> list) -> hash_code {
    hash_code code{0};

    for (const hash_code& h: list) {
      code = crab::hash_code_mix(code, h);
    }

    return code;
  }

  /// Utility to convert an arbitrary amount of hash codes (or hash-code like types) into a single one.
  ///
  /// @param values All hash codes to combine
  /// @return Combined hash code
  template<into_hash_code... Ts>
  [[nodiscard]] CRAB_INLINE constexpr auto hash_code_mix(const Ts&... values) -> hash_code {
    return crab::hash_code_mix(std::initializer_list<hash_code>{crab::hash(values)...});
  }

  /// Combines all the hash codes produced by hashing each of the individual elements.
  /// This is a relatively simple function for quickly getting a hash value using multiple items.
  ///
  /// @param items Items to hash together
  /// @return Combined hash code
  template<ty::hashable... T>
  [[nodiscard]] constexpr auto hash_together(const T&... items) -> hash_code {
    return crab::hash_code_mix({crab::hash(items)...});
  }
}

/// }@
