#pragma once

#include <type_traits>

#include <crab/preamble.hpp>

namespace crab {

  /**
   * @brief Hash code type
   */
  using hash_code = usize;

  /**
   * @brief Any type that can be converted to a hash_code (usize)
   */
  template<typename T>
  concept into_hash_code = std::convertible_to<T, hash_code>;

  /**
   * @brief Is the given type hashable
   */
  template<typename T>
  concept hashable = requires(const T& v) {
    { std::hash<T>{}(v) } -> std::convertible_to<hash_code>;
  };

  template<hashable T>
  inline constexpr auto hash(T&& value) -> hash_code {
    return static_cast<hash_code>(std::hash<T>{}(value));
  }

  constexpr auto hash_code_mix( //
    const hash_code seed,
    const hash_code next
  ) -> hash_code {
    return (next + 0x9e3779b9 + (seed << 6) + (seed >> 2)) ^ seed;
  }

  template<into_hash_code T, into_hash_code... Rest>
  constexpr auto hash_code_mix( //
    T&& first,
    Rest&&... rest
  ) -> hash_code {
    return crab::hash_code_mix(
      first,
      crab::hash_code_mix<Rest...>(std::forward<Rest>(rest)...)
    );
  }

  template<hashable FirstItem>
  constexpr auto hash_together(FirstItem&& first) -> hash_code {
    return crab::hash<FirstItem>(std::forward<FirstItem>(first));
  }

  template<hashable T, hashable... Rest>
  constexpr auto hash_together( //
    T&& first,
    Rest&&... items
  ) -> hash_code {
    return crab::hash_code_mix(
      crab::hash<T>(std::forward<T>(first)),
      crab::hash_together<Rest...>(std::forward<Rest>(items)...)
    );
  }
}
