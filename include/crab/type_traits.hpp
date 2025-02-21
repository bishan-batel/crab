//
// Created by bishan_ on 4/25/24.
//

#pragma once
#include <concepts>
#include <type_traits>
#include <utility>

template<typename T>
class Option;

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

  template<typename F, typename... T>
  using clean_invoke_result =
    std::remove_cvref_t<std::invoke_result_t<F, T...>>;

  namespace option {
    template<typename T>
    struct is_option_type : std::false_type {};

    template<typename T>
    struct is_option_type<Option<T>> : std::true_type {};
  } // namespace option

  template<typename T> concept option_type = option::is_option_type<T>::value;
} // namespace crab
