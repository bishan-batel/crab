#pragma once

#include <type_traits>
#include "preamble.hpp"

namespace crab {

  /**
   * @brief Hash code type
   */
  using hash_code = usize;

  /**
   * @brief Is the given type hashable
   */
  template<typename T> concept hashable = requires(const T& v) {
    { std::hash<T>{v}() } -> std::convertible_to<hash_code>;
  };

  /**
   * @brief Hasher function type
   */
  template<class T, std::invocable<const T&> F>
  requires std::convertible_to<std::invoke_result_t<F, const T&>, hash_code>
  struct hash_function : F {};
}

namespace std {

  template<typename T>
  requires std::convertible_to<
    decltype(T::static_hasher),
    crab::hash_function<T(const T&), crab::hash_code>>
  struct hash<T> /* NOLINT */ {
    constexpr auto operator()(const T& value) const -> crab::hash_code {
      return static_cast<crab::hash_code>(T::static_hasher)(value);
    }
  };

}
