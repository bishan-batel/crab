#pragma once

#include <functional>

#include "crab/ty/classify.hpp"
#include "crab/ty/construct.hpp"

namespace crab::ty {

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
    { std::invoke<F, Args...>(std::forward<F>(function), std::forward<Args>(args)...) } -> convertible<ReturnType>;
  };

  /**
   * @brief Trait for some given type that is callable with the arguments 'Args'
   *
   * Fn(Args...) -> any
   */
  template<typename F, typename... Args>
  concept consumer = requires(F&& function, Args&&... args) {
    std::invoke<F, Args...>(std::forward<F>(function), std::forward<Args>(args)...);
  };

  /**
   * @brief Gets the result of a functor from the given atguments. Ex. for a
   * given mapping function F(Args...) -> Y, this metafunction will take in 'F'
   * and 'Args...' and evaluate to the type 'Y'.
   *
   * @tparam F Functor to check against
   * @tparam Args
   */
  template<typename F, typename... Args>
  requires consumer<F, Args...>
  using functor_result = std::invoke_result_t<F, Args...>;

  /**
   * @brief The same as crab:ty::consumer, with the added constraint that
   * the functor must result in a value that is not void.
   */
  template<typename F, typename Arg, typename... Other>
  concept mapper = consumer<F, Arg, Other...> and not_void<functor_result<F, Arg, Other...>>;

  namespace impl {
    /**
     * @class generator_default
     * @brief Internal crab struct for detecting when a default argument was
     * passed, this should not typically be used externally.
     */
    struct generator_default final {};
  }

  /**
   * @brif Trait for a functor that simply generates a value
   */
  template<typename F, typename Return = impl::generator_default>
  concept generator =
    (same_as<Return, impl::generator_default> ? consumer<F> : functor<F, Return>) and not_void<functor_result<F>>;

  /**
   * @brif Trait for a functor that simply generates a *single* value of F
   *
   * If given a return type, this will also enforce that this should be a
   * provider for that type
   */
  template<typename F, typename Return = impl::generator_default>
  concept provider = non_const<F> and generator<F, Return>;

  /**
   * @brief Trait for a functor that takes in some arguments of type 'Args' and
   * returns a boolean, typically for use as a filter
   */
  template<typename F, typename Arg, typename... Other>
  concept predicate = functor<F, bool, Arg, Other...>;

}
