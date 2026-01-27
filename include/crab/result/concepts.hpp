#pragma once

#include "crab/result/forward.hpp"
#include "crab/ty/bool_types.hpp"
#include "crab/ty/construct.hpp"

namespace crab::result {

  namespace impl {
    template<typename>
    struct is_result_type final : ty::false_type {};

    template<typename T, typename E>
    struct is_result_type<::crab::result::Result<T, E>> final : ty::true_type {};
  }

  /**
   * Type predicate for if the given type T is some form of crab Result
   */
  template<typename T>
  concept result_type = impl::is_result_type<T>::value;

  /**
   * @brief A valid error type for use in Err<T> / Result<_, E>
   */
  template<typename E>
  concept error_type = ty::movable<E>;

  /**
   * @brief Type constraint for a type that can be used with Result<T>
   */
  template<typename T>
  concept ok_type = ty::movable<T>;

}
