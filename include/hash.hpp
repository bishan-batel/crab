#pragma once

#include <preamble.hpp>
#include <type_traits>

namespace crab {

  /**
   * @brief Hash code type
   */
  using hash_code = usize;

  /**
   * @brief Any type that can be converted to a hash_code (usize)
   */
  template<typename T> concept into_hash_code =
    std::convertible_to<T, hash_code>;

  /**
   * @brief Is the given type hashable
   */
  template<typename T> concept hashable = requires(const T& v) {
    { std::hash<T>{}(v) } -> std::convertible_to<hash_code>;
  };

  template<hashable T>
  constexpr auto hash(const T& value) -> hash_code {
    return static_cast<hash_code>(std::hash<T>{}(value));
  }

  constexpr auto hash_code_mix(const hash_code seed, const hash_code next)
    -> hash_code {
    return (next + 0x9e3779b9 + (seed << 6) + (seed >> 2)) ^ seed;
  }

  template<into_hash_code First, into_hash_code... Args>
  constexpr auto hash_code_mix(const First& first, const Args&... rest)
    -> hash_code {
    return crab::hash_code_mix(first, crab::hash_code_mix(rest...));
  }

  template<hashable FirstItem>
  constexpr auto hash_together(const FirstItem& first) -> hash_code {
    return crab::hash(first);
  }

  template<hashable FirstItem, hashable... Items>
  constexpr auto hash_together(const FirstItem& first, const Items&... items)
    -> hash_code {
    return crab::hash_code_mix(
      crab::hash(first),
      crab::hash_together<Items...>(items...)
    );
  }
}
