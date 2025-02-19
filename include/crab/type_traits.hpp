//
// Created by bishan_ on 4/25/24.
//

#pragma once
#include <concepts>
#include <type_traits>
#include <utility>

namespace crab {
  namespace ref {
    /**
     * @brief True if the given type is valid for use in Ref/RefMut<T>
     */
    template<typename T> concept is_valid_type = not std::is_const_v<T>
                                             and not std::is_reference_v<T>
                                             and not std::is_volatile_v<T>;
  }

  /**
   * @brief True if the given type is not a forward declaration
   */
  template<typename T> concept is_complete_type = requires { sizeof(T); };

} // namespace crab
