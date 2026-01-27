#pragma once

#include <type_traits>

namespace crab::ty {
  /**
   * @brief Metafunction for turning const T& or T& -> T, or leave T alone if T
   * is not a reference
   */
  template<typename T>
  using remove_reference = std::remove_reference_t<T>;

}
