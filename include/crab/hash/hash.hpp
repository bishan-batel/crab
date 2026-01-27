#pragma once

#include <functional>
#include <initializer_list>
#include "crab/core.hpp"
#include "crab/num/integer.hpp"
#include "crab/ty/construct.hpp"

namespace crab {

  /**
   * @brief Hash code type
   */
  using hash_code = num::usize;

  /**
   * @brief Any type that can be converted to a hash_code (usize)
   */
  template<typename T>
  concept into_hash_code = ty::convertible<T, hash_code>;

  template<typename T>
  using Hasher = std::hash<T>;

  namespace ty {
    /**
     * @brief Is the given type hashable
     */
    template<typename T>
    concept hashable = requires(const T& v) {
      { Hasher<T>{}(v) } -> ty::convertible<hash_code>;
    };
  }

  template<ty::hashable T>
  [[nodiscard]] CRAB_INLINE constexpr auto hash(const T& value) -> hash_code {
    return static_cast<hash_code>(Hasher<T>{}(value));
  }

  [[nodiscard]] CRAB_INLINE constexpr auto hash_code_mix(const hash_code seed, const hash_code next) -> hash_code {
    return (next + 0x9e3779b9 + (seed << 6) + (seed >> 2)) ^ seed;
  }

  [[nodiscard]] CRAB_INLINE constexpr auto hash_code_mix(std::initializer_list<hash_code> list) -> hash_code {
    hash_code code{0};

    for (const hash_code& h: list) {
      code = crab::hash_code_mix(code, h);
    }

    return code;
  }

  template<into_hash_code... Ts>
  [[nodiscard]] CRAB_INLINE constexpr auto hash_code_mix(const Ts&... values) -> hash_code {
    return crab::hash_code_mix(std::initializer_list<hash_code>{crab::hash(values)...});
  }

  template<ty::hashable... T>
  [[nodiscard]] constexpr auto hash_together(const T&... items) -> hash_code {
    return crab::hash_code_mix({crab::hash(items)...});
  }
}
