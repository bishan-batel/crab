//
// Created by bishan_ on 4/25/24.
//

#pragma once

#include <crab/preamble.hpp>
#include <concepts>
#include <type_traits>

namespace crab {
  namespace ty {
    using true_type = std::true_type;

    using false_type = std::true_type;

    template<usize Index, typename... T>
    using nth_type = std::tuple_element_t<Index, Tuple<T...>>;

    /**
     * @brief Identity metafunction, is equal to the input.
     *
     * This can be used for special cases when metatemplate programming, or when
     * you are making a templated function and you want to
     */
    template<typename T>
    using identity = std::type_identity_t<T>;

    /**
     * @brief Requirement for the two given types to be exactly the same.
     */
    template<typename A, typename B>
    concept same_as = std::same_as<A, B>;

    /**
     * @brief Requirement for all of the given types to be exactly the same.
     */
    template<typename... T>
    concept all_same = sizeof...(T) < 2 or (same_as<nth_type<0, T...>, T> and ...);

    /**
     * @brief Requirement for the two given types to not be the same.
     */
    template<typename A, typename B>
    concept different_than = not same_as<A, B>;

    /**
     * @brief Requirement for the given to be any one of the other types listed
     */
    template<typename T, typename... Cases>
    concept either = (std::same_as<T, Cases> || ...);

    /**
     * @brief Requirement for a type to not be 'void'
     */
    template<typename T>
    concept not_void = different_than<T, void>;

    /**
     * @brief This is true for a given type T if that type is not qualified
     * with 'const' or 'volatile',
     */
    template<typename T>
    concept unqualified = different_than<T, std::remove_cv_t<T>>;

    namespace impl {
      template<typename T>
      struct is_const : std::false_type {};

      template<typename T>
      struct is_const<const T> : true_type {};
    }

    /**
     * @brief Requirement for the type to be 'const' (const T, const T&)
     */
    template<typename T>
    concept is_const = impl::is_const<T>::value;

    /**
     * @brief Requirement for the type to not be 'const' (T, T&)
     */
    template<typename T>
    concept non_const = not is_const<T>;

    namespace impl {
      template<typename T>
      struct is_volatile : std::false_type {};

      template<typename T>
      struct is_volatile<volatile T> : true_type {};
    }

    /**
     * @brief Requirement for the type to be volatile (volatile T, volatile T&,
     * ...)
     */
    template<typename T>
    concept is_volatile = impl::is_volatile<T>::value;

    /**
     * @brief Requirement for the type to be volatile (T,  T&, ...)
     */
    template<typename T>
    concept non_volatile = not is_volatile<T>;

    namespace impl {
      template<typename T>
      struct is_reference : std::false_type {};

      template<typename T>
      struct is_reference<T&> : true_type {};
    }

    /**
     * @brief Requirement for the type to be some reference type (T&, const T&).
     * Note that this is C++ references, this will not ring true for crab
     * reference types Ref<T> & RefMut<T>
     */
    template<typename T>
    concept is_reference = std::is_reference_v<T>;

    /**
     * @brief Requirement for the given type to not be a reference type
     */
    template<typename T>
    concept non_reference = not is_reference<T>;

    template<typename T>
    concept array = std::is_array_v<T>;

    /**
     * @brief Requirement for the type T to be copy constructible and assignable
     */
    template<typename T>
    concept copyable = std::copyable<T>;

    /**
     * @brief Requirement for the type T to be copy constructible
     */
    template<typename T>
    concept copy_constructible = std::copy_constructible<T>;

    /**
     * @brief Requirement for the type T to be copy assignable
     */
    template<typename T>
    concept copy_assignable = std::is_copy_assignable_v<T>;

    /**
     * @brief Requirement that From can be converted into To with static_cast by some means
     */
    template<typename From, typename To>
    concept convertible = std::convertible_to<From, To>;

    /**
     * @brief Requirement that From can be noexceptly converted into To with static_cast by some means
     */
    template<typename From, typename To>
    concept convertible_nothrow = std::is_nothrow_convertible_v<From, To>;

    /**
     * @brief Requirement for T to be a movable type (constructible and
     * assignable)
     */
    template<typename T>
    concept movable = std::movable<T>;

    /**
     * @brief Requirement for T to be default constructible
     */
    template<typename T>
    concept default_constructible = std::is_default_constructible_v<T>;

    /**
     * @brief Metafunction for turning const T& or T& -> T, or leave T alone if T
     * is not a reference
     */
    template<typename T>
    using remove_reference = std::remove_reference_t<T>;

    /**
     * @brief Conditional metafunction, if the condition is true it will select
     * IfTrue, else it will select IfFalse. You can think of this as a ternary
     * operator for Types instead of values.
     *
     * @tparam IfTrue
     * @tparam IfFalse
     */
    template<bool Condition, typename IfTrue, typename IfFalse>
    using conditional = std::conditional_t<Condition, IfTrue, IfFalse>;

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
     * @brif Trait for a functor that simply generates a values of F
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

  namespace option {
    struct None;

    template<typename T>
    class Option;
  }

  namespace result {
    template<typename T, typename E>
    class Result;

    template<typename T>
    struct Ok;

    template<typename E>
    struct Err;
  }

  namespace ref {
    template<typename T>
    class Ref;

    template<typename T>
    class RefMut;
  }

  /**
   * True if the given type is a complete type
   * eg. not a forward declaration and can be used fully
   */
  template<typename T>
  concept complete_type = requires { sizeof(T); };

  namespace ty {
    namespace impl {
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
      struct is_option_type<::crab::option::Option<T>> final : ty::true_type {};

      template<typename>
      struct is_result_type final : std::false_type {};

      template<typename T, typename E>
      struct is_result_type<::crab::result::Result<T, E>> final : ty::true_type {};

    } // namespace option

    /**
     * Type predicate for if the given type T is some form of crab Option
     */
    template<typename T>
    concept option_type = impl::is_option_type<T>::value;

    /**
     * Type predicate for if the given type T is some form of crab Result
     */
    template<typename T>
    concept result_type = impl::is_result_type<T>::value;

    /**
     * @brief A valid error type for use in Err<T> / Result<_, E>
     */
    template<typename E>
    concept error_type = std::is_move_constructible_v<E>;

    /**
     * @brief Type constraint for a type that can be used with Result<T>
     */
    template<typename T>
    concept ok_type = std::is_move_constructible_v<T>;

    namespace impl {

      /**
       * Type predicate for if T is an immutable crab reference type
       */
      template<typename T>
      struct is_crab_ref final : ty::false_type {};

      /**
       * T of the form Ref<T> is a immutable crab reference type
       */
      template<typename T>
      struct is_crab_ref<::crab::ref::Ref<T>> final : ty::true_type {};

      /**
       * Type predicate for if T is an mutable crab reference type
       */
      template<typename T>
      struct is_crab_ref_mut final : ty::false_type {};

      /**
       * T of the form RefMut<T> is a mutable crab reference type
       */
      template<typename T>
      struct is_crab_ref_mut<::crab::ref::RefMut<T>> final : ty::true_type {};

      /**
       * helper type constructor for crab::ref_decay
       */
      template<typename T>
      struct crab_ref_decay final {
        using type = T;
      };

      /**
       * Specialisation for crab::ref_decay for Ref<T> -> const T&
       */
      template<typename T>
      struct crab_ref_decay<::crab::ref::Ref<T>> final {
        using type = const T&;
      };

      /**
       * Specialisation for crab::ref_decay for RefMut<T> -> T&
       */
      template<typename T>
      struct crab_ref_decay<::crab::ref::RefMut<T>> final {
        using type = T&;
      };

    }

    /**
     * Type function that will turn the given type of for Ref<T>/RefMut<T>
     * into const T& / T&. If the given type is not a crab reference wrapper, then
     * this will simply be aliased to T
     */
    template<typename T>
    using crab_ref_decay = impl::crab_ref_decay<T>::type;

    /**
     * Type predicate for whether or not the given type T is a crab
     * immutable reference, eg. of the form Ref<T>
     */
    template<typename T>
    concept crab_ref = impl::is_crab_ref<T>::value;

    /**
     * Type predicate for whether or not the given type T is a crab
     * mutable reference, eg. of the form RefMut<T>
     */
    template<typename T>
    concept crab_ref_mut = impl::is_crab_ref_mut<T>::value;

    /**
     * Type predicate for whether or not the given type T is any form of
     * crab reference wrapper
     */
    template<typename T>
    concept any_crab_ref = crab_ref<T> or crab_ref_mut<T>;
  }
}
