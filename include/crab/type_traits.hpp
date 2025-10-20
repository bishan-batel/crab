//
// Created by bishan_ on 4/25/24.
//

#pragma once

#include <concepts>
#include <type_traits>

#include <crab/preamble.hpp>

template<typename T>
class Option;

template<typename T, typename E>
class Result;

namespace crab {
  template<typename T>
  struct Ok;

  template<typename E>
  struct Err;

  namespace ref {
    /**
     * True if the given type is valid for use in Ref/RefMut<T>
     */
    template<typename T>
    concept is_valid_type =
      not std::is_const_v<T> and not std::is_reference_v<T>;
  }

  /**
   * True if the given type is a complete type
   * eg. not a forward declaration and can be used fully
   */
  template<typename T>
  concept complete_type = requires { sizeof(T); };

  namespace helper {
    /**
     * Type predicate helper for if the given type T is an option type, false
     * for most types
     */
    template<typename>
    struct is_option_type : std::false_type {};

    /**
     * Type predicate helper for if the given type is an option type, true
     * only if the given type is of the form Option<T> for some T
     */
    template<typename T>
    struct is_option_type<Option<T>> : std::true_type {};

    template<typename>
    struct is_result_type : std::false_type {};

    template<typename T, typename E>
    struct is_result_type<Result<T, E>> : std::true_type {};

  } // namespace option

  /**
   * Type predicate for if the given type T is some form of crab Option
   */
  template<typename T>
  concept option_type = helper::is_option_type<T>::value;

  /**
   * Type predicate for if the given type T is some form of crab Result
   */
  template<typename T>
  concept result_type = helper::is_result_type<T>::value;

} // namespace crab

template<crab::ref::is_valid_type T>
class Ref;

template<crab::ref::is_valid_type T>
class RefMut;

namespace crab {
  namespace helper {

    /**
     * Type predicate for if T is an immutable crab reference type
     */
    template<typename T>
    struct is_crab_ref : std::false_type {};

    /**
     * T of the form Ref<T> is a immutable crab reference type
     */
    template<typename T>
    struct is_crab_ref<Ref<T>> : std::true_type {};

    /**
     * Type predicate for if T is an mutable crab reference type
     */
    template<typename T>
    struct is_crab_ref_mut : std::false_type {};

    /**
     * T of the form RefMut<T> is a mutable crab reference type
     */
    template<typename T>
    struct is_crab_ref_mut<RefMut<T>> : std::true_type {};

    /**
     * helper type constructor for crab::ref_decay
     */
    template<typename T>
    struct ref_decay {
      using type = T;
    };

    /**
     * Specialisation for crab::ref_decay for Ref<T> -> const T&
     */
    template<typename T>
    struct ref_decay<Ref<T>> {
      using type = const T&;
    };

    /**
     * Specialisation for crab::ref_decay for RefMut<T> -> T&
     */
    template<typename T>
    struct ref_decay<RefMut<T>> {
      using type = T&;
    };

  }

  /**
   * Type function that will turn the given type of for Ref<T>/RefMut<T>
   * into const T& / T&. If the given type is not a crab reference wrapper, then
   * this will simply be aliased to T
   */
  template<typename T>
  using ref_decay = helper::ref_decay<T>::type;

  /**
   * Type predicate for whether or not the given type T is a crab
   * immutable reference, eg. of the form Ref<T>
   */
  template<typename T>
  concept crab_ref = helper::is_crab_ref<T>::value;

  /**
   * Type predicate for whether or not the given type T is a crab
   * mutable reference, eg. of the form RefMut<T>
   */
  template<typename T>
  concept crab_ref_mut = helper::is_crab_ref_mut<T>::value;

  /**
   * Type predicate for whether or not the given type T is any form of
   * crab reference wrapper
   */
  template<typename T>
  concept any_crab_ref = crab_ref<T> or crab_ref_mut<T>;

}

namespace crab::ty {
  template<typename F, typename... Args>
  concept consumer = std::invocable<F, Args...>;

  template<typename F, typename ReturnType, typename... Args>
  concept functor = std::invocable<F, Args...>;
}
