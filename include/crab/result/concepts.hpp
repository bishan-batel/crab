#pragma once

#include "crab/result/forward.hpp"
#include "crab/ty/bool_types.hpp"
#include "crab/ty/classify.hpp"
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
  concept error_type = ty::movable<E> or ty::is_reference<E>;

  /**
   * @brief Type constraint for a type that can be used with Result<T>
   */
  template<typename T>
  concept ok_type = ty::movable<T> or ty::is_reference<T>;

  namespace impl {
    /**
     * @brief Whether type T is the 'Ok' wrapper Ok<K>
     */
    template<typename>
    struct is_crab_ok final : ty::false_type {};

    /**
     * @brief Whether type T is the 'Ok' wrapper Ok<K>
     */
    template<ok_type T>
    struct is_crab_ok<Ok<T>> final : ty::true_type {};

    /**
     * @brief Whether type T is the 'Err' wrapper Err<K>
     */
    template<typename>
    struct is_crab_err final : ty::false_type {};

    /**
     * @brief Whether type T is the 'Err' wrapper Err<K>
     */
    template<error_type E>
    struct is_crab_err<Err<E>> final : ty::true_type {};
  }

  /**
   * Type predicate for if the given type is of the form crab::Ok<T>
   */
  template<typename T>
  concept crab_ok = impl::is_crab_ok<T>::value;

  /**
   * Type predicate for if the given type is of the form crab::Err<T>
   */
  template<typename T>
  concept crab_err = impl::is_crab_err<T>::value;

}
