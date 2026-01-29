/// @file functor.hpp
/// @ingroup ty
/// Crab type constraints for functors

/// @addtogroup ty
/// @{

#pragma once

#include <functional>

#include "crab/ty/classify.hpp"
#include "crab/ty/construct.hpp"

namespace crab::ty {

  /// Trait for any type that is callable with the argument types 'Args'
  /// and returns the type 'ReturnType'.
  ///
  /// F(Args...) -> ReturnType
  ///
  /// @tparam F Funcor
  /// @tparam ReturnType Type that F(Args...) should ignore
  /// @tparam Args arguments the functor needs to be able to take
  template<typename F, typename ReturnType, typename... Args>
  concept functor = requires(F&& function, Args&&... args) {
    { std::invoke<F, Args...>(std::forward<F>(function), std::forward<Args>(args)...) } -> convertible<ReturnType>;
  };

  /// Trait for some given type that is callable with the arguments 'Args'
  ///
  /// F(Args...) -> any
  ///
  /// @tparam Functor to check against
  /// @tparam Args arguments the functor needs to be able to take
  template<typename F, typename... Args>
  concept consumer = requires(F&& function, Args&&... args) {
    std::invoke<F, Args...>(std::forward<F>(function), std::forward<Args>(args)...);
  };

  /// Gets the result of a functor from the given arguments. Ex. for a
  /// given mapping function F(Args...) -> Y, this metafunction will take in 'F'
  /// and 'Args...' and evaluate to the type 'Y'.
  ///
  /// @tparam F Functor to check against
  /// @tparam Args Arguments the functor needs to be able to take
  template<typename F, typename... Args>
  requires consumer<F, Args...>
  using functor_result = std::invoke_result_t<F, Args...>;

  /// The same as crab:ty::consumer, with the added constraint that
  /// the functor must result in a value that is not void.
  ///
  /// @tparam F Functor to check against
  /// @tparam Arg The argument the mapper must be able to take
  /// @tparam Other Additional arguments the mapper is supposed to take.
  template<typename F, typename Arg, typename... Other>
  concept mapper = consumer<F, Arg, Other...> and not_void<functor_result<F, Arg, Other...>>;

  namespace impl {
    /// @internal
    /// Internal crab struct for detecting when a default argument was
    /// passed, this should not typically be used externally.
    struct generator_default final {};
  }

  /// Trait for a functor that simply generates a value
  ///
  /// @tparam f functor to check against
  /// @tparam Return the type that this function should generate, if not specified then this will allow any nonvoid type
  template<typename F, typename Return = impl::generator_default>
  concept generator = (same_as<Return, impl::generator_default> ? consumer<F> : functor<F, Return>)
                  and not_void<functor_result<F>> and not_void<Return>;

  /// Trait for a functor that simply generates a *single* value of F
  ///
  /// Alias of generator, meant for clarity
  ///
  /// @tparam F functor to check against
  /// @tparam Return the type that this function should generate, if not specified then this will allow any nonvoid type
  template<typename F, typename Return = impl::generator_default>
  concept provider = generator<F, Return>;

  /// Trait for a functor that takes in some arguments of type 'Args' and
  /// returns a boolean. Typically for use as a filter.
  ///
  /// @tparam F functor to check against
  /// @tparam Arg Argument the predicate needs to take
  /// @tparam Other Additional arguments you want the predicate to be required to take.
  template<typename F, typename Arg, typename... Other>
  concept predicate = functor<F, bool, Arg, Other...>;
}

/// }@
