//
// Created by bishan_ on 4/25/24.
//

#pragma once

#include <concepts>
#include <type_traits>

#include <crab/preamble.hpp>

namespace crab::ty {

  /**
   * @brief Identity metafunction, is equal to the input.
   *
   * This can be used for special cases when metatemplate programming, or when
   * you are making a templated function and you want to
   */
  template<typename T>
  using identity = std::type_identity_t<T>;

  /**
   * @brief Type extractor for the result type of a given functor 'F' called
   * with arguments 'Args'
   */
  template<typename F, typename... Args>
  using functor_result = std::invoke_result_t<F>;

  template<typename A, typename B>
  concept same_as = std::same_as<A, B>;

  template<typename A, typename B>
  concept different_than = not same_as<A, B>;

  template<typename T, typename... Cases>
  concept either = (std::same_as<T, Cases> || ...);

  template<typename T>
  concept not_void = different_than<T, void>;

  /**
   * @brief This is true for a given type T if that type is not qualified
   * with 'const', 'volatile', or
   */
  template<typename T>
  concept unqualified = different_than<T, std::remove_cv_t<T>>;

  template<typename T>
  concept is_const = std::is_const_v<T>;

  template<typename T>
  concept non_const = not is_const<T>;

  template<typename T>
  concept is_volatile = std::is_volatile_v<T>;

  template<typename T>
  concept non_volatile = not is_volatile<T>;

  template<typename T>
  concept is_reference = std::is_reference_v<T>;

  template<typename T>
  concept non_reference = not is_reference<T>;

  /**
   * @brief Trait for any type that is callable with the argument types 'Args'
   * and returns the type 'ReturnType'
   *
   * Fn(Args...) -> ReturnType
   *
   * @tparam F Funcor
   * @tparam ReturnType Type that F(Args...) should ignore
   * @tparam Args
   */
  template<typename F, typename ReturnType, typename... Args>
  concept functor = requires(F&& function, Args&&... args) {
    {
      std::invoke<F, Args...>(
        std::forward<F>(function),
        std::forward<Args>(args)...
      )
    } -> std::convertible_to<ReturnType>;
  };

  /**
   * @brief Trait for some given type that is callable with the arguments 'Args'
   *
   * Fn(Args...) -> any
   */
  template<typename F, typename... Args>
  concept consumer = requires(F&& function, Args&&... args) {
    std::invoke<F, Args...>(
      std::forward<F>(function),
      std::forward<Args>(args)...
    );
  };

  template<typename F, typename... Args>
  using mapper_codomain = functor_result<F, Args...>;

  /**
   * @brief The same as crab:ty::consumer, with the added constraint that
   * the functor must result in a value that is not void.
   */
  template<typename F, typename... Args>
  concept mapper =
    consumer<F, Args...> and not_void<mapper_codomain<F, Args...>>;

  /**
   * @brif Trait for a functor that simply generates a values of F
   */
  template<typename F, typename Return>
  concept generator = functor<F, Return>;

  /**
   * @brif Trait for a functor that simply generates a *single* value of F
   *
   * This is almost the same as generator, however this is supposed to be used
   * for semantic readability
   */
  template<typename F, typename Return>
  concept provider = not std::is_const_v<F> and generator<F, Return>;

  /**
   * @brief Trait for a functor that takes in some arguments of type 'Args' and
   * returns a boolean, typically for use as a filter
   */
  template<typename F, typename Arg, typename... Other>
  concept predicate = functor<F, bool, Arg, Other...>;

}

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
    concept is_valid_type = ty::non_const<T> and ty::non_reference<T>;
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
    struct is_option_type final : std::false_type {};

    /**
     * Type predicate helper for if the given type is an option type, true
     * only if the given type is of the form Option<T> for some T
     */
    template<typename T>
    struct is_option_type<Option<T>> final : std::true_type {};

    template<typename>
    struct is_result_type final : std::false_type {};

    template<typename T, typename E>
    struct is_result_type<Result<T, E>> final : std::true_type {};

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
    struct is_crab_ref final : std::false_type {};

    /**
     * T of the form Ref<T> is a immutable crab reference type
     */
    template<typename T>
    struct is_crab_ref<Ref<T>> final : std::true_type {};

    /**
     * Type predicate for if T is an mutable crab reference type
     */
    template<typename T>
    struct is_crab_ref_mut final : std::false_type {};

    /**
     * T of the form RefMut<T> is a mutable crab reference type
     */
    template<typename T>
    struct is_crab_ref_mut<RefMut<T>> final : std::true_type {};

    /**
     * helper type constructor for crab::ref_decay
     */
    template<typename T>
    struct ref_decay final {
      using type = T;
    };

    /**
     * Specialisation for crab::ref_decay for Ref<T> -> const T&
     */
    template<typename T>
    struct ref_decay<Ref<T>> final {
      using type = const T&;
    };

    /**
     * Specialisation for crab::ref_decay for RefMut<T> -> T&
     */
    template<typename T>
    struct ref_decay<RefMut<T>> final {
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
